#include "crash_logger.h"
#include "enumerate_sessions.h"
#include <mmdeviceapi.h>

session enumerate_sessions::Enumarate_Audio_Session(int index) {
	session buff;
	HRESULT hr = 0;
	if (pSessionEnum == nullptr)
		reinitialize();
	hr = pSessionEnum->GetSession(index, &pSessionControl);
	hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&buff.control);
	if (hr != S_OK) {
		printf("Was unable to aquire simpleaudiovolume controller!\n");
	}
	buff.name = Get_Session_Display_Name(&pSessionControl);
	buff.control->GetMasterVolume(&buff.initial_volume);   // TODO: Check if this actually returns the valid value.
	//buff.control->GetMute((BOOL*)&buff.mute);  // ERROR: This gives a access violation error...
	if (buff.name == "NULL") {
		buff.name = Get_Session_Display_Name(&pSessionControl);
		if (buff.name == "NULL") {
			std::string text = "(HRESULT)ERROR CODE " + std::to_string(hr);
			crash_logger ci;
			ci.log_message(text + ", Getting session display name", __FUNCTION__);
			buff.name = "UNKNOWN";
		}
	}
	//std::remove_if(buff.name.begin(), buff.name.end(), ::isspace);
	buff.name.erase(std::remove_if(buff.name.begin(), buff.name.end(), ::isspace), buff.name.end());
	return buff;
}

std::string enumerate_sessions::Get_Session_Display_Name(IAudioSessionControl** pSessionControl) {
	HRESULT hr = S_OK;
	LPWSTR try_Str = 0;
	std::string display;  //What to display.
	if ((*pSessionControl) == NULL) {
		crash_logger cl;
		cl.log_message_with_last_error("Somehow the session controller was null...", __FUNCTION__);
		return "NULL";
	}
	/*
	Tries to retrieve the display name from the actual controller object, this usually returns NULL
	but there is one particular session that windows does not use(atleast for app volumes), 
	"AudioSrv". The only way to find it was to get the display name. hence the find and exclude, directly.
	*/
	(*pSessionControl)->GetDisplayName(&try_Str);
	std::wstring temp = try_Str;
	if (temp.find(L"AudioSrv") == std::wstring::npos) {
		IAudioSessionControl2* PID;  //For the the PID(process id)
		(*pSessionControl)->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&PID);
		DWORD max = MAX_PATH;
		DWORD t2 = 0;
		char buffer[MAX_PATH];

		hr = PID->GetProcessId(&t2);
		if (hr != S_OK) {
			crash_logger ci;
			ci.log_message_with_last_error(std::to_string(t2) + std::string("Could not get processID!"), __FUNCTION__);
		}

		HANDLE hprocess = NULL;

		if (t2 != 0)
			hprocess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, t2);

		if (hprocess != NULL) {
			if (QueryFullProcessImageName(hprocess, 0, buffer, &max) == 0) { // Will get the actual process path(including the NAME).
				crash_logger ci;
				ci.log_message_with_last_error(std::string("Could not get process name!"), __FUNCTION__);
			}
			display = buffer;
			try {
				display.erase(0, display.find_last_of("\\") + 1);  //Removes the path, excluding the progam name.
				display.erase(display.size() - 4, display.size());  // --||--
			}
			/*
			If the string erase failed, dont throw a error and exit, 
			just log the problem, and return "UNPARSE" as the display name...
			*/
			catch (std::exception e) {
				printf("%s\n", e.what());
				crash_logger ci;
				ci.log_message(e.what() + std::string("Could not parse the audio session name"), __FUNCTION__);
				display = "UNPARSE";
			}
		}
		else {
			return "NULL";
		}

		std::transform(display.begin(), display.end(), display.begin(), ::toupper);
		std::string buff = "";
		/*
		Old way of removing a space from a string... fun, hip, cool
		*/
		for (char c : display) {
			if (c != (char)32)
				buff += c;
		}

		display = buff;

		/*
		The maximum size of a string is 7 characters, 
		because of the limited size of the LCD screen
		*/
		if (display.size() > 6)
			display.erase(6, display.size());
		else if (display.size() < 6) {
			int save_Size = display.size();
			for (int a = 0; a < 6 - save_Size; a++) {
				if (a % 2 == 0) {
					display = display + " ";
				}
				else if (a % 2 == 1) {
					display = " " + display;
				}
			}
		}
		SAFE_RELEASE(PID);
		if (hprocess != NULL)
			CloseHandle(hprocess);
	}
	else {
		display = "SYSTEM";
	}
	return display;
}


HRESULT enumerate_sessions::CreateSessionManager(IAudioSessionManager2** ppSessionManager) {
	HRESULT hr = S_OK;

	IMMDevice* pDevice = NULL;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IAudioSessionManager2* pSessionManager = NULL;
	hr = CoInitialize(pEnumerator);
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

	if (hr != S_OK) {
		printf("Could not CoCreateInstance\n");
		crash_logger cl;
		cl.log_message_with_last_error("Could not CoCreateInstance", __FUNCTION__);
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pSessionManager);
		return hr;
	}

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
	if (hr != S_OK)
		return hr;
	hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);
	if (hr != S_OK)
		return hr;

	*(ppSessionManager) = pSessionManager;
	(*ppSessionManager)->AddRef();

	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pSessionManager);
	return hr;
}