#pragma once
#include <curl/curl.h>
#include <string>
class Network_Functionality {
private:
	std::string last_Mod;
	bool check_BETA = false;
	std::string version;
public:
	Network_Functionality();
	bool Check_For_Updates(bool quiet_Mode);
	bool Download_Updates();
public:
	void set_Version(std::string v) { version = v; }
};

