#include "Global_Variables.h"

namespace global {
	std::string com_Port = "COM6";  //BASE COM VALUE...
	std::string update_Time = "1997-10-11T19:57:47";
	extern std::string documents_Settings_Path = "";
	volatile int page_Number = 1;
	std::vector<std::string> programs;
	std::vector<std::string> pref/* = std::vector<std::string>(36)*/;
	bool waiting_Update = false;
	bool enable_logging = false;
	std::vector<std::string> to_Exclude;
	//std::vector<std::string> prefered_list = std::vector<std::string>(36);
	std::vector<reserved> reserved_List;
	std::vector<std::string> rename_list;
	std::vector<rename_session> renamed;
}