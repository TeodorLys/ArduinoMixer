#pragma once
#include "enumerate_device.h"
#include <ArduinoConnect.hpp>

class initialize_device {
private:
	enumerate_device _e_device;
	self::Arduino_API arduino;
	std::string found_port;
public:
	initialize_device();
	/*
	Rechecks the registry for any new connected devices, and checks them against the saved container id.
	Or if no container id are saved, it will connect to all found ports and do a info call
	SEE: device_IO - CALL_COMMANDS enum.
	*/
	void reenumeration();
	/*
	Checks all enumerated devices against the saved container id
	*/
	void find_device_by_container_id();

	/*
	Connects to all enumerated devices and does a info call.
	SEE: device_IO - CALL_COMMANDS enum.
	*/
	void retry_devices();
};

