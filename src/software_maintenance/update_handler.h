#pragma once
#include "network_handler.h"
class update_handler {
private:
	network_handler net;
	std::string update_path;
public:
	bool check_for_updates(bool quiet_mode = false);
	bool download_updates();
	bool install_updates();
	bool finish_and_clean_install();
	bool install_device_firmware(std::string firmware);
};

