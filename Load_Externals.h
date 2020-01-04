#pragma once
#include <string>
#include <SaveAble.h>
#include <libconfig.h++>
#include <iostream>
#include "crash_logger.h"

class Load_Externals {
private:
		//SABool *logging = nullptr;
		//SABool *waiting = nullptr;
		//SAString *com = nullptr;
		//SAString *netUpdate = nullptr;
		//SAVariable_Array *exclude = nullptr;
		//SAVariable_Array *prefered = nullptr;
		//SAVariable_Array *rename = nullptr;
		//SAVariable_Array *page_option = nullptr;
		//SAString *ci_keycombo = nullptr;
		//SABool *auto_update_check = nullptr;
		static libconfig::Config cfg;
		//SaveSettings *settings = nullptr;
		static std::string documents_Path;   //Path to the current user documents folder
		static std::string settings_Path; 
		static bool is_Logging;  // Default file values...
		static bool system_Control;   // --||--
		const int file_Entrys = 5; // If this does not match the number of settings in the file it will recreate it...
private:
	void Load_File();
	void Create_File();
	void _Get_COM_From_Reg();
	void Settings_Load();
	void Settings_Save_Variables();
	void rewrite_all_data();
	
public:
	Load_Externals();
	void Try_Load_Externals(bool update);
	void Update_Save_File();
	void Complete_Update_Preload_File();
	void Complete_Update_Save_File();
	void Construct_Reserved_List();
	void Construct_Rename_List();
	void Parse_Document_Path();
public:
	bool get_Logging() const { return is_Logging; }
	bool get_Exclude_System() const { return system_Control; }
	void write_to_file() {
		try {
			cfg.writeFile(settings_Path.c_str());
		}
		catch (libconfig::FileIOException &e) {
			printf("could not load file, exiting, %s\n", e.what());
			crash_logger logger;
			logger.log_message(e.what(), __FUNCTION__);
			std::cin.get();
			exit(1);
		}
	}
};

