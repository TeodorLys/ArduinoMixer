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
	std::vector<std::string> PIDs;
	//Default vid + pid number of a Teensy 3.2
	std::string _def_serial_nr = "16C00483";

protected:
	void enumerate_guid_ports();
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

	size_t get_container_size() {
		return devices.size();
	}

	std::string get_container_id(int index) {
		if (index < devices.size()) {
			return devices[index].containerid;
		}
		return "NULL";
	}

	std::string get_container_port(int index) {
		if (index < devices.size()) {
			return devices[index].comport;
		}
		return "NULL";
	}
};

