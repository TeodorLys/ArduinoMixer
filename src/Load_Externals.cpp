#include "Global_Variables.h"
#include "Load_Externals.h"
#include "Network_Functionality.h"
#include "crash_logger.h"
#include "device_IO.h"
#include <string>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <SaveAble.h>
#include <ArduinoConnect.hpp>
#include <OGUI_Window/keyboard_handler.h>

libconfig::Config Load_Externals::cfg;
std::string Load_Externals::documents_Path = "";
std::string Load_Externals::settings_Path = "";
bool Load_Externals::is_Logging = false;
bool Load_Externals::system_Control = false;

Load_Externals::Load_Externals() {
	//logging = new SABool("Enable_Logging", &is_Logging);
	//waiting = new SABool("Waiting_Update", &global::waiting_Update);
	//com = new SAString("Com_Value", &device_IO::com_port());
	//netUpdate = new SAString("Last_Modified", &Network_Functionality::last_update());
	//exclude = new SAVariable_Array("Exclude_In_Volume_Control", &global::to_Exclude);
	//prefered = new SAVariable_Array("Prefered_List", &global::pref);
	//rename = new SAVariable_Array("Rename_List", &global::rename_list);
	//page_option = new SAVariable_Array("Page_Options", &global::page_options);
	//ci_keycombo = new SAString("Chrome_Key_Combo", &global::ci_key);
	//auto_update_check = new SABool("Auto_update_check", &global::auto_update);

}

void Load_Externals::Construct_Reserved_List() {
	int t = 0;
	for (int a = 0; a < (int)global::pref.size(); a++) {
		if (global::pref[a] != "UNUSED") {
			printf("Found reserved, %i\n", a);
			global::reserved_List.push_back(reserved(a, global::pref[a], global::_page_options[t]._option_index));
		}
		if(a % 4 == 3){
			t++;
		}
	}
}

void Load_Externals::Construct_Rename_List() {
	rename_session buff;
	for (int a = 0; a < (int)global::rename_list.size(); a++) {
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
	documents_Path = std::filesystem::temp_directory_path().string();
	try {
		documents_Path.erase(documents_Path.size() - 1, documents_Path.size());
		documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
		documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
		documents_Path.erase(documents_Path.find_last_of("\\"), documents_Path.size());
		documents_Path += "\\Documents";
	}
	catch (std::exception e) {
		printf("%s\n", e.what());
		MessageBox(NULL, "For some reason I could not parse the documents path... Does it exist?", "FATAL ERROR HAS OCCURED", MB_OK);
		crash_logger ci;
		ci.log_message(e.what(), __FUNCTION__);
		exit(0);
	}
	if (!std::filesystem::exists(documents_Path + "\\ArduinoMixerData")) {
		try {
			std::filesystem::create_directory(documents_Path + "\\ArduinoMixerData");
		}
		catch (const std::filesystem::filesystem_error e) {
			printf("%s\n", e.code().message().c_str());
			crash_logger ci;
			ci.log_message(e.what(), __FUNCTION__);
			exit(0);
		}
	}

	global::documents_Settings_Path = documents_Path + "\\ArduinoMixerData";
}



void Load_Externals::Try_Load_Externals(bool update) {
	settings_Path = documents_Path + "\\ArduinoMixerData\\settings.ini";
	//settings = new SaveSettings(settings_Path, false);
	if (std::filesystem::exists(settings_Path) && !update) {
		Load_File();
	}
	else if(!update) {
		Create_File();
	}
	Construct_Reserved_List();
	Construct_Rename_List();
}

void Load_Externals::Settings_Load() {
	//settings->Load(*logging);
	//settings->Load(*waiting);
	//settings->Load(*com);
	//settings->Load(*netUpdate);
	//settings->Load(*exclude);
	//settings->Load(*prefered);
	//settings->Load(*rename);
	//settings->Load(*ci_keycombo);
	//settings->Load(*auto_update_check);
	//settings->Load(*page_option);


	libconfig::Setting& root = cfg.getRoot();

	if (!root.exists("system")) {
		rewrite_system_data();
	}

	if (!root.exists("basic")) {
		rewrite_all_data();
		return;
	}
	libconfig::Setting& basic = root["basic"];
	libconfig::Setting& system = root["system"];

	if(!basic.exists("Enable_Logging"))
		basic.add("Enable_Logging", libconfig::Setting::TypeBoolean) = is_Logging;
	else
		basic.lookupValue("Enable_Logging", is_Logging);
	if (!basic.exists("Waiting_Update"))
		basic.add("Waiting_Update", libconfig::Setting::TypeBoolean) = global::waiting_Update;
	else 
		basic.lookupValue("Waiting_Update", global::waiting_Update);
	if(!basic.exists("Last_Update"))
		basic.add("Last_Update", libconfig::Setting::TypeString) = Network_Functionality::last_update();
	else
		basic.lookupValue("Last_Update", Network_Functionality::last_update());
	if (!basic.exists("Exclude_List")) {
		libconfig::Setting& exclude = basic.add("Exclude_List", libconfig::Setting::TypeArray);
		for (int a = 0; a < (int)global::to_Exclude.size(); a++)
			exclude.add(libconfig::Setting::TypeString) = global::to_Exclude[a];
	}
	else {
		libconfig::Setting& exclude = basic.lookup("Exclude_List");
		for (int a = 0; a < exclude.getLength(); a++) {
			global::to_Exclude.push_back(exclude[a]);
		}
	}

	if (!basic.exists("Prefered_List")) {
		libconfig::Setting& pref = basic.add("Prefered_List", libconfig::Setting::TypeArray);
		for (int a = 0; a < (int)global::pref.size(); a++)
			pref.add(libconfig::Setting::TypeString) = global::pref[a];
	}
	else {
		libconfig::Setting& pref = basic.lookup("Prefered_List");
		for (int a = 0; a < pref.getLength(); a++) {
			global::pref.push_back(pref[a]);
		}
	}

	if (!basic.exists("Rename_List")) {
		libconfig::Setting& rename = basic.add("Rename_List", libconfig::Setting::TypeArray);
		for (int a = 0; a < (int)global::rename_list.size(); a++)
			rename.add(libconfig::Setting::TypeString) = global::rename_list[a];
	}
	else {
		libconfig::Setting& rename = basic.lookup("Rename_List");
		for (int a = 0; a < rename.getLength(); a++) {
			global::rename_list.push_back(rename[a]);
		}
	}

	if (!basic.exists("Page_Option_List")) {
		libconfig::Setting& page = basic.add("Page_Option_List", libconfig::Setting::TypeArray);
		for (int a = 0; a < (int)global::page_options.size(); a++)
			page.add(libconfig::Setting::TypeString) = global::page_options[a];
	}
	else {
		libconfig::Setting& page = basic.lookup("Page_Option_List");
		for (int a = 0; a < page.getLength(); a++) {
			global::page_options.push_back(page[a]);
		}
	}
	if(!basic.exists("Chrome_key_combo"))
		basic.add("Chrome_key_combo", libconfig::Setting::TypeString) = global::ci_key;
	else
		basic.lookupValue("Chrome_key_combo", global::ci_key);
	if (!basic.exists("Auto_Update"))
		basic.add("Auto_Update", libconfig::Setting::TypeBoolean) = global::auto_update;
	else 
		basic.lookupValue("Auto_Update", global::auto_update);

	if (!system.exists("Container_ID"))
		system.add("Container_ID", libconfig::Setting::TypeString) = global::container_id;
	else
		system.lookupValue("Container_ID", global::container_id);

	if (!system.exists("Com_Port"))
		system.add("Com_Port", libconfig::Setting::TypeString) = device_IO::com_port();
	else
		system.lookupValue("Com_Port", device_IO::com_port());

	write_to_file();
}

void Load_Externals::Settings_Save_Variables() {
	//settings->Save_Variables(*logging, f);
	//settings->Save_Variables(*waiting, f);
	//settings->Save_Variables(*com, f);
	//settings->Save_Variables(*netUpdate, f);
	//settings->Save_Variables(*exclude, f);
	//settings->Save_Variables(*prefered, f);
	//settings->Save_Variables(*rename, f);
	//settings->Save_Variables(*ci_keycombo, f);
	//settings->Save_Variables(*auto_update_check, f);
	//settings->Save_Variables(*page_option, f);


	libconfig::Setting& root = cfg.getRoot();
	libconfig::Setting& basic = root["basic"];
	libconfig::Setting& system = root["system"];

	basic["Enable_Logging"] = is_Logging;
	basic["Waiting_Update"] = global::waiting_Update;
	basic["Last_Update"] = Network_Functionality::last_update();
	basic.remove("Exclude_List");
	libconfig::Setting& exclude = basic.add("Exclude_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::to_Exclude.size(); a++)
		exclude.add(libconfig::Setting::TypeString) = global::to_Exclude[a];
	basic.remove("Prefered_List");
	libconfig::Setting& pref = basic.add("Prefered_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::pref.size(); a++)
		pref.add(libconfig::Setting::TypeString) = global::pref[a];
	basic.remove("Rename_List");
	libconfig::Setting& rename = basic.add("Rename_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::rename_list.size(); a++)
		rename.add(libconfig::Setting::TypeString) = global::rename_list[a];
	basic.remove("Page_Option_List");
	libconfig::Setting& page = basic.add("Page_Option_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::page_options.size(); a++)
		page.add(libconfig::Setting::TypeString) = global::page_options[a];
	basic["Chrome_key_combo"] = global::ci_key;
	basic["Auto_Update"] = global::auto_update;

	system["Com_Port"] = device_IO::com_port();
	system["Container_ID"] = global::container_id;

	write_to_file();
}

void Load_Externals::write_system_data() {
	libconfig::Setting& root = cfg.getRoot();
	if (!root.exists("system")) {
		rewrite_system_data();
		return;
	}
	libconfig::Setting& system = root["system"];
	system["Com_Port"] = device_IO::com_port();
	system["Container_ID"] = global::container_id;
	write_to_file();
}

void Load_Externals::Load_File() {
	//std::fstream file(settings_Path, std::fstream::in);
	try {
		cfg.readFile(settings_Path.c_str());
	}
	catch (libconfig::FileIOException &e) {
		printf("Could not read the configuration file, exiting... %s\n", e.what());
		crash_logger ci;
		ci.log_message(+ e.what(), __FUNCTION__);
		std::cin.get();
		exit(1);
	}
	Settings_Load();
	//settings->Load_Settings();
	//if (settings->get_last_error() != "") {
	//	crash_logger ci;
	//	ci.log_message(settings->get_last_error(), __FUNCTION__);
	//	exit(0);
	//}
	global::enable_logging = is_Logging;

	printf("%s\n", global::ci_key.c_str());

	if (global::ci_key.find("+") != std::string::npos) {
		std::string addition = global::ci_key;
		std::string key = global::ci_key;
		try {
			addition.erase(global::ci_key.find("+"), global::ci_key.length());
			key.erase(0, global::ci_key.find("+") + 1);
			printf("%s\n", key.c_str());
			global::ci_key_hex = ogui::keyboard_handler::string_to_enum(key);
			global::ci_add_hex = ogui::keyboard_handler::string_to_enum(addition);
		}
		catch (std::exception &e) {
			printf("could not parse keypress, %s\n", e.what());
			crash_logger ci;
			ci.log_message(e.what(), __FUNCTION__);
		}
	}
	else {
		global::ci_key_hex = ogui::keyboard_handler::string_to_enum(global::ci_key);
	}
	if (global::pref.size() == 0) {
		for (int a = 0; a < 36; a++) {
			global::pref.push_back("UNUSED");
		}
	}
	if (global::page_options.size() == 0) {
		printf("is_zero\n");
		for (int a = 0; a < 9; a++) {
			std::string buff = "0;" + std::to_string(a) + ";";
			global::page_options.push_back(buff);
		}
		Update_Save_File();
	}
	for (int a = 0; a < (int)global::page_options.size(); a++) {
		/*
		There are probably 100 better ways to do this, but hey it works! JANQ ftw
		*/
		try {
			std::string nindex = global::page_options[a];
			nindex.erase(nindex.find(";"), nindex.length());
			std::string npage = global::page_options[a];
			npage.erase(0, npage.find(";") + 1);
			npage.pop_back();
			int page_num = std::stoi(npage);
			int option_num = std::stoi(nindex);
			global::_page_options.push_back(page_spec(page_num, option_num, global::pref[(page_num * 4) + 0], global::pref[(page_num * 4) + 1], global::pref[(page_num * 4) + 2], global::pref[(page_num * 4) + 3]));
		}
		catch (std::exception & e) {
			crash_logger ci;
			ci.log_message(e.what(), __FUNCTION__);
		}
	}
}

void Load_Externals::Create_File() {
	Network_Functionality nf;
	nf.Check_For_Updates(true);
	global::to_Exclude.push_back("SYSTEM");
	global::to_Exclude.push_back("SHELLEX");
	std::fstream file(settings_Path, std::fstream::out);
//	_Get_COM_From_Reg();
	file.close();
	global::enable_logging = is_Logging;
	for (int a = 0; a < 36; a++) {
		global::pref.push_back("UNUSED");
	}

	libconfig::Setting& root = cfg.getRoot();
	libconfig::Setting& basic = root.add("basic", libconfig::Setting::TypeGroup);
	basic.add("Enable_Logging", libconfig::Setting::TypeBoolean) = is_Logging;
	basic.add("Waiting_Update", libconfig::Setting::TypeBoolean) = global::waiting_Update;
	basic.add("Last_Update", libconfig::Setting::TypeString) = Network_Functionality::last_update();
	libconfig::Setting &exclude = basic.add("Exclude_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::to_Exclude.size(); a++)
		exclude.add(libconfig::Setting::TypeString) = global::to_Exclude[a];
	libconfig::Setting &pref = basic.add("Prefered_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::pref.size(); a++)
		pref.add(libconfig::Setting::TypeString) = global::pref[a];
	libconfig::Setting &rename = basic.add("Rename_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::rename_list.size(); a++)
		rename.add(libconfig::Setting::TypeString) = global::rename_list[a];
	libconfig::Setting &page = basic.add("Page_Option_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::page_options.size(); a++)
		page.add(libconfig::Setting::TypeString) = global::page_options[a];

	basic.add("Chrome_key_combo", libconfig::Setting::TypeString) = global::ci_key;
	basic.add("Auto_Update", libconfig::Setting::TypeBoolean) = global::auto_update;

	libconfig::Setting& system = root.add("system", libconfig::Setting::TypeGroup);
	system.add("Com_Port", libconfig::Setting::TypeString) = device_IO::com_port();
	system.add("Container_ID", libconfig::Setting::TypeString) = global::container_id;

	write_to_file();

	//file.open(settings_Path, std::fstream::out);
	//Settings_Save_Variables(&file);
	//file.close();
}

void Load_Externals::rewrite_all_data() {
	if (global::to_Exclude.size() == 0) {
		global::to_Exclude.push_back("SYSTEM");
		global::to_Exclude.push_back("SHELLEX");
	}

	libconfig::Setting& root = cfg.getRoot();
	libconfig::Setting& basic = root.add("basic", libconfig::Setting::TypeGroup);
	basic.add("Enable_Logging", libconfig::Setting::TypeBoolean) = is_Logging;
	basic.add("Waiting_Update", libconfig::Setting::TypeBoolean) = global::waiting_Update;
	basic.add("Last_Update", libconfig::Setting::TypeString) = Network_Functionality::last_update();
	libconfig::Setting& exclude = basic.add("Exclude_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::to_Exclude.size(); a++)
		exclude.add(libconfig::Setting::TypeString) = global::to_Exclude[a];
	libconfig::Setting& pref = basic.add("Prefered_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::pref.size(); a++)
		pref.add(libconfig::Setting::TypeString) = global::pref[a];
	libconfig::Setting& rename = basic.add("Rename_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::rename_list.size(); a++)
		rename.add(libconfig::Setting::TypeString) = global::rename_list[a];
	libconfig::Setting& page = basic.add("Page_Option_List", libconfig::Setting::TypeArray);
	for (int a = 0; a < (int)global::page_options.size(); a++)
		page.add(libconfig::Setting::TypeString) = global::page_options[a];

	basic.add("Chrome_key_combo", libconfig::Setting::TypeString) = global::ci_key;
	basic.add("Auto_Update", libconfig::Setting::TypeBoolean) = global::auto_update;
	rewrite_system_data();
	write_to_file();
}

void Load_Externals::rewrite_system_data() {
	libconfig::Setting& root = cfg.getRoot();
	libconfig::Setting& system = root.add("system", libconfig::Setting::TypeGroup);
	system.add("Com_Port", libconfig::Setting::TypeString) = device_IO::com_port();
	system.add("Container_ID", libconfig::Setting::TypeString) = global::container_id;
}

void Load_Externals::Complete_Update_Preload_File() {
	Settings_Load();
	//settings->Load_Settings();
	//if (settings->get_last_error() != "") {
	//	crash_logger ci;
  //	ci.log_message(settings->get_last_error(), __FUNCTION__);
	//	exit(0);
	//}
	global::enable_logging = is_Logging;
}

void Load_Externals::Complete_Update_Save_File() {
	if (global::to_Exclude.size() == 0) {
		global::to_Exclude.push_back("SYSTEM");
		global::to_Exclude.push_back("SHELLEX");
	}
	std::fstream file(settings_Path, std::fstream::out);
	Settings_Save_Variables();
	file.close();
}

void Load_Externals::Update_Save_File() {
	is_Logging = global::enable_logging;
	Settings_Save_Variables();
}

void Load_Externals::_Get_COM_From_Reg() {
	self::Arduino_API api;
	printf("checking_coms...");
	device_IO::com_port() = api.Get_Specific_Arduino_Port(1000000, "AM", "~");
	printf("DONE!\n");
	api.disconnect();
}