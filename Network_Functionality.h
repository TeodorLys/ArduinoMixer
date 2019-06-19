#pragma once
#include <curl/curl.h>
#include <string>

class Network_Functionality {
private:
	std::string last_Mod;
	bool check_BETA = false;
	std::string version;
	HANDLE h_mutex;

	size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream) {
		((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
		return size * nmemb;
	}
public:
	Network_Functionality();
	bool Check_For_Updates(bool quiet_Mode);
	bool Download_Updates();
	void Give_Mutex_Handle(HANDLE h) { h_mutex = h; }
public:
	void set_Version(std::string v) { version = v; }
	static bool _has_internet_connection() {
		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();

		if (curl) {
			curl_easy_setopt(curl, CURLOPT_URL, "www.google.com");
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_data);
			res = curl_easy_perform(curl);
			if (res != CURLE_OK) {
				curl_easy_cleanup(curl);
				return false;
			}
			else {
				curl_easy_cleanup(curl);
				return true;
			}
			
		}
		return false;
	}
};

