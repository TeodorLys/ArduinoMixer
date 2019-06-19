#pragma once
#include <string>
#include <SaveAble.h>

class Network_Functionality;

class Load_Externals {
private:
		SABool *logging;
		SABool *waiting;
		SAString *com;
		SAString *netUpdate;
		SAVariable_Array *exclude;
		SAVariable_Array *prefered;
		SAVariable_Array *rename;
		SaveSettings *settings;
		std::string documents_Path = "";   //Path to the current user documents folder
		std::string settings_Path = ""; 
		bool is_Logging = false;  // Default file values...
		bool system_Control = false;   // --||--
		const int file_Entrys = 5; // If this does not match the number of settings in the file it will recreate it...
		Network_Functionality *nf;
private:
	void Load_File();
	void Create_File();
	void _Get_COM_From_Reg();
	void Settings_Load();
	void Settings_Save_Variables(std::fstream *f);
	
public:
	Load_Externals(Network_Functionality *n);
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
	~Load_Externals() {
		delete logging;
		delete waiting;
		delete com;
		delete netUpdate;
		delete exclude;
		delete prefered;
		delete rename;
		delete settings;
	}
};

