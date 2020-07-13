#include "Global_Variables.h"
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <curl/curl.h>
#include "Network_Functionality.h"
#include "Zip_File_Handler.h"
#include "BalloonTip.h"
#include <myio.h>

void Start_Program(LPCSTR app) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	CreateProcess(app, NULL, NULL, NULL, TRUE, DETACHED_PROCESS, NULL, NULL, &si, &pi);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}

size_t wwrite_data(void* ptr, size_t size, size_t nmemb, void* stream) {
	((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
	return size * nmemb;
}


size_t write_new_data(void* buffer, size_t size, size_t nmemb, void* userp) {
	return size * nmemb;
}

std::string Network_Functionality::version;
HANDLE Network_Functionality::h_mutex;
std::string Network_Functionality::_last_update;

/*
This is for the EXTRA functionallity, for the pots. @Deprecated, Can't be used anymore, protocol changed
*/
void Network_Functionality::send_request(int r, int g, int b) {
	CURL* curl;
	CURLcode res;
	curl = curl_easy_init();
	std::string url = "http://192.168.86.164/action_page.php?RED=" + std::to_string(r);
	url += "&GREEN=" + std::to_string(g);
	url += "&BLUE=" + std::to_string(b);
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_new_data);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			printf("Could not connect to the internet!\n");
			return;
		}
	}
	curl_easy_cleanup(curl);
}
/*------------------*/

void Network_Functionality::Download_single_file(std::string _filename, std::string _outfile) {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	std::string out;
	std::string args = "Dropbox-API-Arg: {\"path\":\"/";  // Dropbox:es api, default stuff...

	/*
	If the request was not successful, I still want the program to continue.
	Butt? if it was, cleanup and create the file.
	*/
	bool success = false;

	args += _filename + "\"}";
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
		//yes i know... should really not use a "static" access token, but github was not very reliable...
		list = curl_slist_append(list, "Authorization: Bearer vADqFIv5AnQAAAAAAAAADE7r9DJpeLInASfi6o5H5KsdyryX7RJRQqpL5IAusLGE");
		list = curl_slist_append(list, "Content-Type:");
		list = curl_slist_append(list, args.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &wwrite_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			printf("Could not connect to the internet!\n");
			BalloonTip::Call_BallonTip("CONNECTION ERROR", "Could not connect to the internet");
			BalloonTip::_Set_Identifier(2);
		}
		else {
			success = true;
		}
	}
	else {
		printf("Could not open curl\n");
	}
	if (success) {
		curl_easy_cleanup(curl);
		curl_global_cleanup();
		std::fstream f;
		f.open(_outfile, std::fstream::binary | std::fstream::out);
		f << out;
		f.close();
	}
}

bool Network_Functionality::Download_Updates() {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	std::string out;
	std::string args = "Dropbox-API-Arg: {\"path\":\"/";

	if (_last_update.find(".zip") == std::string::npos) {
		_last_update += ".zip";
	}

	args += _last_update + "\"}";
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
		//yes i know... really shouldnt not use a "static" access token, but github was not very reliable...
		list = curl_slist_append(list, "Authorization: Bearer vADqFIv5AnQAAAAAAAAADE7r9DJpeLInASfi6o5H5KsdyryX7RJRQqpL5IAusLGE");
		list = curl_slist_append(list, "Content-Type:");
		list = curl_slist_append(list, args.c_str());

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &wwrite_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			printf("Could not connect to the internet!\n");
			BalloonTip::Call_BallonTip("CONNECTION ERROR", "Could not connect to the internet");
			BalloonTip::_Set_Identifier(2);
			return false;
		}
	}
	else {
		printf("Could not open curl\n");
	}
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	/*
	A zip file always start with P and K, should probably use some sort of hash, but...
	*/
	if (out[0] == 'P' && out[1] == 'K') {
		std::string temp_Path = std::filesystem::temp_directory_path().string();
		std::fstream f;
		f.open(temp_Path + "\\" + _last_update, std::fstream::binary | std::fstream::out);
		f << out;
		f.close();

		Zip_File_Handler zip;
		std::string zip_file = temp_Path;
		
		zip_file += _last_update;
		zip.Open_Zip(zip_file.c_str());
		
		std::string unzip_path = temp_Path;
		
		zip.Unzip(unzip_path.c_str());

		DeleteFile(zip_file.c_str());

		std::fstream curr_File(temp_Path + "curr_temp_file.txt", std::fstream::binary | std::fstream::out);
		curr_File << std::filesystem::current_path().string() << std::endl;
		curr_File << temp_Path << std::endl;
		curr_File << global::documents_Settings_Path << std::endl;
		curr_File.close();

		std::string updater = temp_Path + "Updater.exe";
		printf("%s\n", updater.c_str());
		ShellExecute(NULL, "runas", updater.c_str(), 0, 0, SW_SHOWNORMAL);
		ReleaseMutex(h_mutex);
		exit(0);
	}
	else {
		printf("Could not find the file...\n");
		return false;
	}
	return false;
}

bool Network_Functionality::Check_For_Updates(bool quiet_Mode) {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	std::string out;
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
		list = curl_slist_append(list, "Authorization: Bearer vADqFIv5AnQAAAAAAAAADE7r9DJpeLInASfi6o5H5KsdyryX7RJRQqpL5IAusLGE");
		list = curl_slist_append(list, "Content-Type:");
		list = curl_slist_append(list, "Dropbox-API-Arg: {\"path\":\"/VERSION.txt\"}");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &wwrite_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			if (quiet_Mode) {
				printf("Could not connect to the internet!\n");
				BalloonTip::Call_BallonTip("CONNECTION ERROR", "Could not connect to the internet");
				BalloonTip::_Set_Identifier(2);
			}
			return false;
		}
	}

	curl_global_cleanup();

	std::string temp;

	for (char c : out) {
		if (c != '\n')
			temp += c;
		else
			break;
	}
	std::string server_Version = temp;
	int iversion = myio::stoiEX(version) + 1;
	int iserversion = myio::stoiEX(server_Version) + 1;
	printf("%s, %s, %i, %i\n", server_Version.c_str(), version.c_str(), iserversion, iversion);
	if (_last_update == "")
		_last_update = temp;
	else if (iserversion > iversion) {
		printf("Update FOUND!\n");

		BalloonTip::_Set_Identifier(1);
		BalloonTip::Call_BallonTip("UPDATE AVAILABLE!", "Found an update, click to Install");
		_last_update = temp;
		return true;
	}
	else {
		printf("Everything is up to date!\n");
		/*
		If quiet_mode is true, it means that the program desided to check
		for updates(for ex. time based). if it's false, it means that the
		user clicked the "Check for updates" button.
		i.e. no notification will be sent if quiet mode is active!
		*/
		if (!quiet_Mode) {
			BalloonTip::_Set_Identifier(2);
			BalloonTip::Call_BallonTip("Everything is up to date!", "No update found");
		}
		_last_update = temp;
		return false;
	}
	return false;
}