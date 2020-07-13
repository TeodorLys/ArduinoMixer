#include "Global_Variables.h"

namespace global {
	sf::Font font;
	//std::string com_Port = "COM-1";  //BASE COM VALUE...
	//std::string update_Time = "1997-10-11T19:57:47";
	std::string documents_Settings_Path = "";
	std::string ci_key;
	std::string container_id = "{}";
	int version;
	int ci_key_hex;
	int ci_add_hex = 0;
	volatile int page_Number = 1;
	std::vector<std::string> programs;
	std::vector<std::string> pref/* = std::vector<std::string>(36)*/;
	bool waiting_Update = false;
	bool enable_logging = false;
	bool auto_update = true;
	bool prog_ending = false;
	bool experimental = false;
	std::vector<std::string> to_Exclude;
	std::vector<reserved> reserved_List;
	std::vector<std::string> rename_list;
	std::vector<rename_session> renamed;
	std::vector<page_spec> _page_options;
	std::vector<std::string> page_options;
}