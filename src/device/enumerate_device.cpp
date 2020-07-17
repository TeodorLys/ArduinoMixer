#include "enumerate_device.h"
#include <windows.h>
#include <Setupapi.h>
#include <devguid.h>

enumerate_device::enumerate_device() {
	enumerate_guid_ports();
	extract_comport_and_container_id();
}

void enumerate_device::enumerate_guid_ports() {
	/*
	This function will be called multiple times through the programs runtime, so you need to clear the last found devices.
	*/
	PIDs.clear();
	HDEVINFO deviceInfoSet;
	
	GUID* guidDev = (GUID*)&GUID_DEVCLASS_PORTS;
	//Retrieves all connected devices
	deviceInfoSet = SetupDiGetClassDevs(guidDev, NULL, NULL, DIGCF_PRESENT | DIGCF_PROFILE);
	TCHAR buffer[4000];
	DWORD buffersize = 4000;
	int memberIndex = 0;

	while (true) {
		SP_DEVINFO_DATA deviceInfoData;
		ZeroMemory(&deviceInfoData, sizeof(SP_DEVINFO_DATA));
		deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (SetupDiEnumDeviceInfo(deviceInfoSet, memberIndex, &deviceInfoData) == FALSE) {
			if (GetLastError() == ERROR_NO_MORE_ITEMS) {
				break;
			}
		}

		DWORD nSize = 0;
		SetupDiGetDeviceInstanceId(deviceInfoSet, &deviceInfoData, buffer, sizeof(buffer), &nSize);
		buffer[nSize] = '\0';
		std::string buff = buffer;
		PIDs.push_back(buff);
		memberIndex++;
	}

	if (deviceInfoSet) {
		SetupDiDestroyDeviceInfoList(deviceInfoSet);
	}
}

void enumerate_device::extract_comport_and_container_id() {
	std::vector<std::string> found_matches;

	/*
	Formats the vid + pid number and checks it against the connected devices.
	*/
	for (int a = 0; a < (int)PIDs.size(); a++) {
		std::string vid = _def_serial_nr[0].substr(0, 4);
		std::string pid = _def_serial_nr[0].substr(4, 7);

		std::string pid_vid = "VID_" + vid + "&PID_" + pid;
		if (PIDs[a].find(pid_vid) != std::string::npos) {
			found_matches.push_back(PIDs[a]);
		}
		vid = _def_serial_nr[1].substr(0, 4);
		pid = _def_serial_nr[1].substr(4, 7);

		pid_vid = "VID_" + vid + "&PID_" + pid;
		if (PIDs[a].find(pid_vid) != std::string::npos) {
			found_matches.push_back(PIDs[a]);
		}
		pid_vid = _def_serial_nr[2] + "_C00000000";
		if (PIDs[a].find(pid_vid) != std::string::npos) {
			found_matches.push_back(PIDs[a]);
		}
	}


	/*
	Opens the reg key for all found devices, container id and port name(ex. COM1)
	*/
	if (found_matches.size() > 0) {
		for (int a = 0; a < (int)found_matches.size(); a++) {
			HKEY hKey = 0;
			HKEY container_Key = 0;
			std::string key = "SYSTEM\\ControlSet001\\Enum\\" + found_matches[a] + "\\Device Parameters";
			std::string container_key = "SYSTEM\\ControlSet001\\Enum\\" + found_matches[a];
			LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, KEY_READ, &hKey);
			LONG cRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, container_key.c_str(), 0, KEY_READ, &container_Key);

			std::string com_temp;
			std::string container_temp;

			CHAR szBuffer[512];
			CHAR container_szBuffer[512];
			DWORD dwBufferSize = sizeof(szBuffer);
			DWORD container_dwBufferSize = sizeof(container_szBuffer);
			ULONG nError;

			nError = RegQueryValueEx(hKey, "PortName", 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
			if (ERROR_SUCCESS == nError) {
				com_temp = szBuffer;
			}

			nError = RegQueryValueEx(container_Key, "ContainerID", 0, NULL, (LPBYTE)container_szBuffer, &container_dwBufferSize);
			if (ERROR_SUCCESS == nError) {
				container_temp = container_szBuffer;
			}

			if (com_temp != "" && container_temp != "") {
				printf("%s, %s\n", com_temp.c_str(), container_temp.c_str());
				devices.push_back(_devices(container_temp, com_temp));
			}
			RegCloseKey(container_Key);
			RegCloseKey(hKey);
		}
	}
}