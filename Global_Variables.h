#pragma once

/*I know global variables are bad...
but in my defence, I am too lazy to 
come up with some smart solution to share variables*/
#include "Chrome_Integration.h"
#include <SFML/Graphics/Font.hpp>
#include <string>
#include <vector>


struct reserved {
	int index = -1;
	std::string name;

	reserved(int i, std::string s) : index(i), name(s) {}
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

namespace global {
	extern std::string com_Port;
	extern std::string update_Time;
	extern std::string documents_Settings_Path;
	extern volatile int page_Number;
	extern std::vector<std::string> programs;
	extern std::vector<std::string> pref;
	extern bool waiting_Update;
	extern bool enable_logging;
	extern std::vector<std::string> to_Exclude;
	//extern std::vector<std::string> prefered_list;
	extern std::vector<reserved> reserved_List;
	extern std::vector<std::string> rename_list;
	extern std::vector<rename_session> renamed;
	extern Chrome_Integration ci;    // DONT DO THIS!!! TEMPORARY!
	extern sf::Font font;
}