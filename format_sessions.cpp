#include "format_sessions.h"

void format_sessions::reconstruct_sessions() {
	enum_sessions.reinitialize();

	push_sessions_to_buffer();

	for (int a = 0; a < buffer.size(); a++) {
		for (int b = 0; b < 36; b++) {
			if (buffer[a].name == get(b).name || check_For_New_Name(buffer[a].name) == get(b).name) {
				get(b).control = buffer[a].control;
				get(b).initial_volume = buffer[a].initial_volume;
				get(b).mute = buffer[a].mute;
				break;
			}
		}
	}
}

session format_sessions::get_recently_closed_program() {
	enum_sessions.Free_All();
	int count = enum_sessions.Get_Count();

	std::string compare;
	for (int a = 0; a < count; a++) {
		session s = *enum_sessions.Enumarate_Audio_Session(a);
		compare += s.name;
		printf("%s\n", compare.c_str());
	}

	for (int a = 0; a < 36; a++) {
		printf("%s\n", get(a).name.c_str());
		if (compare.find(get(a).name) == std::string::npos && get(a).name != "UNUSED" && get(a).name != "RESERV") {
			return get(a);
		}
	}
}

session format_sessions::get_recently_opened_program() {
	push_sessions_to_buffer();

	std::string compare;
	for (int a = 0; a < 36; a++) {
		if(get(a).name != "UNUSED" && get(a).name != "RESERV")
			compare += get(a).name;
	}

	for (int a = 0; a < buffer.size(); a++) {
		printf("%s\n", get(a).name.c_str());
		if (compare.find(buffer[a].name) == std::string::npos && buffer[a].name != "UNUSED" && buffer[a].name != "RESERV") {
			return buffer[a];
		}
	}
}

session_reservation format_sessions::get_reserv_session(int index) {
	int count = 0;
	session_reservation buff;
	for (int a = 0; a < 36; a++) {
		if (get(a).reserved) {
			if (count != index)
				count++;
			else {
				buff.index = a;
				buff.name = get(a).reserved_name;
				return buff;
			}
		}
	}
	buff.index = -1;
	buff.name = "UNKNOWN";
}