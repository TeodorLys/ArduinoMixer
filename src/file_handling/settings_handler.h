#pragma once
#include <string>
#include <SaveAble.h>
#include <libconfig.h++>
#include <iostream>
#include <ShlObj_core.h>
#include "crash_logger.h"

class settings_handler {
private:
	static libconfig::Config cfg;
	static std::string settings_Path; 
	static bool is_Logging;  // Default file values...
	static bool system_Control;   // --||--
	const int file_Entrys = 5; // If this does not match the number of settings in the file it will recreate it...

public:
	static std::string documents_Path;   //Path to the current user documents folder
private:
	void Load_File();
	void Create_File();
	
	void _Get_COM_From_Reg(); // @Deprecated
	void read_basic_data();
	void write_basic_data();
	void rewrite_system_data();
	void rewrite_all_data();

public:
	settings_handler();
	/*
	It will load information from the settings file, but if no settings file was found it will create it.
	the update parameter, if true it will only construct the reserv and rename list.
	TODO: Move the update parameter function into it's own function...
	*/
	void Try_settings_handler(bool update);
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
	void reread_all_data();
	void write_system_data();
	void read_token_from_file();
public:
	bool get_Logging() const { return is_Logging; }
	bool get_Exclude_System() const { return system_Control; } //@Deprecated
	static std::string get_documents_path() {
		TCHAR path[MAX_PATH];

		SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS, NULL, 0, path);
		return std::string(path);
	}
private:
	void write_to_file() {
		try {
			cfg.writeFile(settings_Path.c_str());
		}
		catch (libconfig::FileIOException &e) {
			printf("could not load file, exiting, %s\n", e.what());
			crash_logger logger;
			logger.log_message(e.what() + std::string("Could not write settings file"), __FUNCTION__);
			std::cin.get();
			exit(1);
		}
	}
};

