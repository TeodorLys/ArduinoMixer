#pragma once
#include <curl/curl.h>
#include <string>

class Network_Functionality {
private:
	bool check_BETA = false;
	static std::string version;
	static HANDLE h_mutex;
	static std::string _last_update;
	size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream) {
		((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
		return size * nmemb;
	}
public:
	/*
	Downloads the version.txt from dropbox, quiet mode is for auto check update.
	*/
	bool Check_For_Updates(bool quiet_Mode);
	bool Download_Updates();
	/*
	Example, the chrome integration files.
	*/
	void Download_single_file(std::string _filename, std::string _outfile);
	/*
	@Deprecated
	*/
	static void send_request(int r, int g, int b);
public:
	static std::string& last_update() {
		return _last_update;
	}
	/*
	For when we update the program, we need to release the mutex, so we can restart the program...
	*/
	void Give_Mutex_Handle(HANDLE h) { h_mutex = h; }
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

