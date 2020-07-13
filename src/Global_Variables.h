#pragma once

/*I know global variables are bad...
but in my defence, I am too lazy to 
come up with some smart solution to share variables*/
#include <SFML/Graphics/Font.hpp>
#include "Chrome_Integration.h"
#include <string>
#include <vector>


struct reserved {
	int index = -1;
	std::string name;
	int _option_index = 0;
	reserved(int i, std::string s, int option_index = 0) : index(i), name(s), _option_index(option_index) {}
};

struct rename_session {
	std::string default_name;
	std::string new_name;

	rename_session(std::string def, std::string newn) : default_name(def), new_name(newn) {}
	rename_session() {}
};

struct AudioSession {
	float value = 1.f;
	int last_value = 0;
	bool has_changed = true;
};

struct page_spec {
	int _page = -1;
	int _option_index = -1;
	std::string _name[4];
	page_spec(int page, int option_index, std::string name1 = "UNUSED", std::string name2 = "UNUSED", std::string name3 = "UNUSED", std::string name4 = "UNUSED") : _page(page), _option_index(option_index) {
		_name[0] = name1;
		_name[1] = name2;
		_name[2] = name3;
		_name[3] = name4;
	}
};

namespace global {
	//extern std::string com_Port;
	//extern std::string update_Time;
	extern std::string documents_Settings_Path;
	extern std::string ci_key; // Chrome integration hotkey
	extern std::string container_id; // Teensy chip container id, got from within the regkey.
	extern int version; 
	extern int ci_key_hex;
	extern int ci_add_hex;
	extern volatile int page_Number;
	extern std::vector<std::string> programs;
	extern std::vector<std::string> pref;
	extern bool waiting_Update;
	extern bool enable_logging;
	extern bool auto_update;
	extern bool prog_ending; // when the program is prepering for closing.
	extern bool experimental;
	extern std::vector<std::string> to_Exclude;
	extern std::vector<reserved> reserved_List;
	extern std::vector<std::string> rename_list;
	extern std::vector<rename_session> renamed;
	extern std::vector<page_spec> _page_options;
	extern std::vector<std::string> page_options;
	extern sf::Font font;  // @Deprecated Should probably remove this...
}