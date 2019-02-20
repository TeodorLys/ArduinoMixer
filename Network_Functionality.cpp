#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <curl/curl.h>
#include "Network_Functionality.h"
#include "Zip_File_Handler.h"
#include "Global_Variables.h"
#include "BalloonTip.h"

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


size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream) {
	((std::string*) stream)->append((const char*)ptr, (size_t)size * nmemb);
	return size * nmemb;
}

Network_Functionality::Network_Functionality()
{
}
bool Network_Functionality::Download_Updates() {
	CURL *curl;
	CURLcode res;
	struct curl_slist *list = 0;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	std::string out;
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, "https://content.dropboxapi.com/2/files/download");
		list = curl_slist_append(list, "Authorization: Bearer <ACCESS TOKEN>");
		list = curl_slist_append(list, "Content-Type:");
		if (check_BETA) 
			list = curl_slist_append(list, "Dropbox-API-Arg: {\"path\":\"/ArduinoMixer_BETA.zip\"}");
		else 
		  list = curl_slist_append(list, "Dropbox-API-Arg: {\"path\":\"/ArduinoMixer.zip\"}");

		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
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

	if (out[0] == 'P' && out[1] == 'K') {
		std::string temp_Path = std::experimental::filesystem::temp_directory_path().string();
		std::fstream f;
		if(check_BETA)
			f.open(temp_Path + "\\ArduinoMixer_BETA.zip", std::fstream::binary | std::fstream::out);
		else
			f.open(temp_Path + "\\ArduinoMixer.zip", std::fstream::binary | std::fstream::out);
		f << out;
		f.close();

		Zip_File_Handler zip;
		std::string zip_file = temp_Path;
		if (check_BETA)
			zip_file += "ArduinoMixer_BETA.zip";
		else 
			zip_file += "ArduinoMixer.zip";
		zip.Open_Zip(zip_file.c_str());
		std::string unzip_path = temp_Path;
		zip.Unzip(unzip_path.c_str());
		std::string s = temp_Path;
		if (check_BETA)
			s += "ArduinoMixer_BETA.zip";
		else
			s += "ArduinoMixer.zip";
		
		DeleteFile(s.c_str());

		std::fstream curr_File(temp_Path + "curr_temp_file.txt", std::fstream::binary | std::fstream::out);
		curr_File << std::experimental::filesystem::current_path().string() << std::endl;
		curr_File << temp_Path << std::endl;
		curr_File << global::documents_Settings_Path << std::endl;
		curr_File.close();

		std::string updater = temp_Path + "Updater.exe";
		printf("%s\n", updater.c_str());
		ShellExecute(NULL, "runas", updater.c_str(), 0, 0, SW_SHOWNORMAL);
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
		list = curl_slist_append(list, "Authorization: Bearer <ACCESS TOKEN>");
		list = curl_slist_append(list, "Content-Type:");
		list = curl_slist_append(list, "Dropbox-API-Arg: {\"path\":\"/VERSION.txt\"}");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
		//if(check_BETA)
		//	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"path\":\"/ArduinoMixer_BETA.zip\"}");
		//else
		//	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"path\":\"/ArduinoMixer.zip\"}");
		res = curl_easy_perform(curl);

		if (res != CURLE_OK) {
			printf("Could not connect to the internet!\n");
			BalloonTip::Call_BallonTip("CONNECTION ERROR", "Could not connect to the internet");
			BalloonTip::_Set_Identifier(2);
			return false;
		}
	}

		curl_easy_cleanup(curl);
	curl_global_cleanup();

	std::string temp;

	for (char c : out) {
		if (c != '\n')
			temp += c;
		else
			break;
	}

	printf("%s, %s\n", temp.c_str(), version.c_str());
	if (global::update_Time == "")
		global::update_Time = temp;
	else if (temp.find(version) == std::string::npos) {
		printf("Update FOUND!\n");
		int answer = 0;
		if (!quiet_Mode) {
			//answer = MessageBox(NULL, "UPDATE FOUND!", "An update is available, want to install?", MB_YESNO);
			BalloonTip::_Set_Identifier(1);
			BalloonTip::Call_BallonTip("UPDATE AVAILABLE!", "Found an update, click to Install");
		}
		global::update_Time = temp;
		global::waiting_Update = true;
		return true;
	}
	else {
		printf("Everything is up to date!\n");
		if (!quiet_Mode) {
			BalloonTip::_Set_Identifier(2);
			BalloonTip::Call_BallonTip("Everything is up to date!", "No update found");
		}
		global::update_Time = temp;
		return false;
	}
	return false;
}