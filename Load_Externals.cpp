#include "Load_Externals.h"
#include "Global_Variables.h"
#include <string>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <SaveAble.h>
#include <ArduinoConnect.hpp>
#include "Network_Functionality.h"

Load_Externals::Load_Externals(Network_Functionality *n) : nf(n){
	logging = new SABool("Enable_Logging", &is_Logging);
	waiting = new SABool("Waiting_Update", &global::waiting_Update);
	com = new SAString("Com_Value", &value);
	netUpdate = new SAString("Last_Modified", &global::update_Time);
	exclude = new SAVariable_Array("Exclude_In_Volume_Control", &global::to_Exclude);
	prefered = new SAVariable_Array("Prefered_List", &global::pref);
	rename = new SAVariable_Array("Rename_List", &global::rename_list);
}

void Load_Externals::Construct_Reserved_List() {
	for (int a = 0; a < (int)global::pref.size(); a++) {
		if (global::pref[a] != "UNUSED") {
			printf("Found reserved, %i\n", a);
			global::reserved_List.push_back(reserved(a, global::pref[a]));
		}
	}
}

void Load_Externals::Construct_Rename_List() {
	rename_session buff;
	for (int a = 0; a < global::rename_list.size(); a++) {
		if (global::rename_list[a].find("->") != std::string::npos) {
			buff.default_name = global::rename_list[a];
			buff.default_name.erase(buff.default_name.find("->"), buff.default_name.length());
			buff.new_name = global::rename_list[a];
			buff.new_name.erase(0, buff.new_name.find("->") + 2);
			global::renamed.push_back(buff);
		}
	}
}

void Load_Externals::Parse_Document_Path() {
	//Undoubtedly a better way to do this... but it is late at night, and other attempts was futile...
	documents_Path = std::experimental::filesystem::temp_directory_path().string();
	documents_Path.erase(documents_Path.size() - 1, documents_Path.size());
	documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
	documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
	documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
	documents_Path += "\\Documents";

	if (!std::experimental::filesystem::exists(documents_Path + "\\ArduinoMixerData")) {
		try {
			std::experimental::filesystem::create_directory(documents_Path + "\\ArduinoMixerData");
		}
		catch (const std::experimental::filesystem::filesystem_error e) {
			printf("%s\n", e.code().message().c_str());
		}
	}

	global::documents_Settings_Path = documents_Path + "\\ArduinoMixerData";
}

void Load_Externals::Try_Load_Externals(bool update) {
	settings_Path = documents_Path + "\\ArduinoMixerData\\settings.ini";
	settings = new SaveSettings(settings_Path, false);
	if (std::experimental::filesystem::exists(settings_Path) && !update) {
		Load_File();
	}
	else if(!update) {
		Create_File();
	}
	Construct_Reserved_List();
	Construct_Rename_List();
}

void Load_Externals::Settings_Load() {
	settings->Load(*logging);
	settings->Load(*waiting);
	settings->Load(*com);
	settings->Load(*netUpdate);
	settings->Load(*exclude);
	settings->Load(*prefered);
	settings->Load(*rename);
}

void Load_Externals::Settings_Save_Variables(std::fstream *f) {
	settings->Save_Variables(*logging, f);
	settings->Save_Variables(*waiting, f);
	settings->Save_Variables(*com, f);
	settings->Save_Variables(*netUpdate, f);
	settings->Save_Variables(*exclude, f);
	settings->Save_Variables(*prefered, f);
	settings->Save_Variables(*rename, f);
}

void Load_Externals::Load_File() {
	std::fstream file(settings_Path, std::fstream::in);
	Settings_Load();
	settings->Load_Settings();
	global::com_Port = value;
	global::enable_logging = is_Logging;
}

void Load_Externals::Create_File() {
	nf->Check_For_Updates(true);
	global::to_Exclude.push_back("SYSTEM");
	global::to_Exclude.push_back("SHELLEX");
	std::fstream file(settings_Path, std::fstream::out);
	_Get_COM_From_Reg();
	file.close();
	global::com_Port = value;
	global::enable_logging = is_Logging;
	for (int a = 0; a < 36; a++) {
		global::pref.push_back("UNUSED");
	}

	file.open(settings_Path, std::fstream::out);
	Settings_Save_Variables(&file);
	file.close();
}

void Load_Externals::Complete_Update_Preload_File() {
	//std::fstream file(settings_Path, std::fstream::in);
	Settings_Load();
	settings->Load_Settings();
	global::com_Port = value;
	global::enable_logging = is_Logging;
	//file.close();
}

void Load_Externals::Complete_Update_Save_File() {
	//nf->Check_For_Updates(true);
	if (global::to_Exclude.size() == 0) {
		global::to_Exclude.push_back("SYSTEM");
		global::to_Exclude.push_back("SHELLEX");
	}
	std::fstream file(settings_Path, std::fstream::out);
	Settings_Save_Variables(&file);
	file.close();
}

void Load_Externals::Update_Save_File() {
	std::fstream file(settings_Path, std::fstream::out);
	is_Logging = global::enable_logging;
	Settings_Save_Variables(&file);
	file.close();
}

void Load_Externals::_Get_COM_From_Reg() {
	/*HKEY hKey = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\ControlSet001\\Enum\\USB\\VID_2341&PID_0243\\554313039373513191B0\\Device Parameters", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		WCHAR szbuffer[512];
		DWORD dwBufferSize = sizeof(szbuffer);
		if (RegQueryValueExW(hKey, L"PortName", 0, NULL, (LPBYTE)szbuffer, &dwBufferSize) == ERROR_SUCCESS) {
			std::wstring temp = szbuffer;
			value = std::string(temp.begin(), temp.end());
		}
		else {
			printf("Found key, but could not find value...\n");
		}
	}
	else {
		printf("Please plug in the Mixer!\n");
		printf("Press any key, when it is plugged in, and driver is installed...");
		std::cin.get();
		Create_File();
	}*/

	self::Arduino_API api;
	value = api.Get_Specific_Arduino_Port(1000000, "AM", "~");
	api.disconnect();
}