#pragma once
#include <string>
#include <SaveAble.h>
#include <libconfig.h++>
#include <iostream>
#include "crash_logger.h"

class Load_Externals {
private:
	/*
	Old save system...
	*/
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
		//SaveSettings *settings = nullptr;
		static libconfig::Config cfg;
		static std::string documents_Path;   //Path to the current user documents folder
		static std::string settings_Path; 
		static bool is_Logging;  // Default file values...
		static bool system_Control;   // --||--
		const int file_Entrys = 5; // If this does not match the number of settings in the file it will recreate it...
private:
	void Load_File();
	void Create_File();
	
	void _Get_COM_From_Reg(); // @Deprecated
	void Settings_Load();
	void Settings_Save_Variables();
	void rewrite_all_data();
	void rewrite_system_data();

public:
	Load_Externals();
	/*
	It will load information from the settings file, but if no settings file was found it will create it.
	the update parameter, if true it will only construct the reserv and rename list.
	TODO: Move the update parameter function into it's own function...
	*/
	void Try_Load_Externals(bool update);
	/*
	If any setting changes, save/update the change
	*/
	void Update_Save_File();
	/*
	Loads/Save all information into its assigned variable
	*/
	void Complete_Update_Preload_File();
	void Complete_Update_Save_File();
	/*
	Parses the save file array into an actual array
	*/
	void Construct_Reserved_List();
	void Construct_Rename_List();
	/*
	finds the system documents path
	*/
	void Parse_Document_Path();
	/*
	Writes the container id and com port to settings file.
	*/
	void write_system_data();
public:
	bool get_Logging() const { return is_Logging; }
	bool get_Exclude_System() const { return system_Control; } //@Deprecated
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

