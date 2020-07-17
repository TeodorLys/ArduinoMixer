#pragma once
#include <vector>
#include <string>
class enumerate_device {
private:
	
	struct _devices {
		std::string containerid;
		std::string comport;
		_devices(std::string container, std::string com) : containerid(container), comport(com){}
	};

	std::vector<_devices> devices;
	std::vector<std::string> PIDs;  // Buffer
	//Default vid + pid number of a Teensy 3.2, Arduino UNO and ESP32 HUZZAH32
	//VID_10C4&PID_EA60
	std::string _def_serial_nr[3] = { "16C00483", "10C4EA60", "30AEA4F2CE86"};

protected:
	/*
	Puts all connected devices into a buffer, using the SetupDiGetClass and SetupDiGetDevice.
	*/
	void enumerate_guid_ports();

	/*
	Parses the buffer created by the enumerate_guid_ports
	*/
	void extract_comport_and_container_id();
public:
	enumerate_device();
	
	/*
	Checks the registy for any connected serial ports
	*/
	void reenumerate_devices() {
		enumerate_guid_ports();
		extract_comport_and_container_id();
	}

	/*
	Returns of how many connected com ports was found
	*/
	size_t get_container_size() {
		return devices.size();
	}


	/*
	Returns the container ID of the devices
	*/
	std::string get_container_id(int index) {
		if (index < (int)devices.size()) {
			return devices[index].containerid;
		}
		return "NULL";
	}

	/*
	Returns the name of the com port ex.(COM4)
	*/
	std::string get_container_port(int index) {
		if (index < (int)devices.size()) {
			return devices[index].comport;
		}
		return "NULL";
	}
};

