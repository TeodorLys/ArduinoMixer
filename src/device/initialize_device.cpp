#include "Global_Variables.h"
#include "initialize_device.h"
#include "device_IO.h"
#include "settings_handler.h"

initialize_device::initialize_device() {}

void initialize_device::reenumeration() {
	_e_device.reenumerate_devices();
	if (global::container_id == "{}")
		retry_devices();
	else
		find_device_by_container_id();
}

void initialize_device::find_device_by_container_id() {
	std::string id = global::container_id;
	printf("Connection by container id: %s\n", id.c_str());

	for (int a = 0; a < (int)_e_device.get_container_size(); a++) {
		if (id == _e_device.get_container_id(a)) {
			printf("Found port: %s\n", _e_device.get_container_port(a).c_str());
			device_IO::com_port() = _e_device.get_container_port(a);
			return;
		}
	}

	retry_devices();

}

void initialize_device::retry_devices() {
	for (int a = 0; a < (int)_e_device.get_container_size(); a++) {
		std::string s = _e_device.get_container_port(a);
		printf("%s, %s\n", _e_device.get_container_id(a).c_str(), _e_device.get_container_port(a).c_str());
		if (_e_device.get_container_port(a).find("\\") == std::string::npos)
			s = "\\\\.\\" + s;
		if (arduino.connect(s.c_str(), 1000000, true)) {
			Sleep(100);
			if (!arduino.writeTo("x", 2)) {
				printf("failed write\n");
				arduino.disconnect();
				continue;
			}
			else{
				char read_buffer[256] = "";
				int i = arduino.recieveFrom(read_buffer, 256);
				if (!i) {
					printf("failed read %s, %i\n", read_buffer, i);
					arduino.disconnect();
					continue;
				}
				else {
					std::string buffer = read_buffer;
					printf("%s\n", buffer.c_str());
					if (buffer.find("Teodor") != std::string::npos) {
						found_port = _e_device.get_container_port(a);
						device_IO::com_port() = found_port;
						global::container_id = _e_device.get_container_id(a);
						settings_handler load;
						load.write_system_data();
					}
				}
			}
		}
		else {
			crash_logger c;
			c.log_message_with_last_error("Could not connect to serial port", __FUNCTION__);
			arduino.disconnect();
		}
		arduino.disconnect();
	}
}
