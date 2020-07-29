#include "format_sessions.h"

void format_sessions::reenumerate_sessions() {
	push_sessions_to_buffer();

	for (int a = 0; a < (int)buffer.size(); a++) {
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

void format_sessions::construct_sessions() {
	push_sessions_to_buffer();

	for (int a = 0; a < 36; a++) {
		if (get(a).reserved) {
			for (int b = 0; b < (int)buffer.size(); b++) {
				if (compare_reserved_session(buffer[b].name, a)) {
					printf("Reserved: %s\n", buffer[b].name.c_str());
					assign_session_spot(a, check_For_New_Name(buffer[b].name));
					if (reg.exists(buffer[b].name))
						reg.update_entity(buffer[b].name, true);
					else 
						reg.add_entity(buffer[b].name, true);
					buffer.erase(buffer.begin() + b);
				}
			}
		}
	}

	for (int a = 0; a < (int)buffer.size(); a++) {
		printf("Session_buffer: %s, %i\n", buffer[a].name.c_str(), a);
		if (Check_for_Restrictions(buffer[a].name.c_str())) {
			if (reg.exists(buffer[a].name))
				reg.update_entity(buffer[a].name);
			else
				reg.add_entity(buffer[a].name);
			assign_next_unused_spot(check_For_New_Name(buffer[a].name));
		}
	}
	reenumerate_sessions();
}

session format_sessions::get_recently_closed_program() {
	enum_sessions.Free_All();
	int count = enum_sessions.Get_Count();

	/*
	I add all the sessions into a single string, which I can
	then can compare against by checking if the OLD session buffer contains
	a name -- if it doesnt return it.
	*/

	std::string compare;
	for (int a = 0; a < count; a++) {
		session s = enum_sessions.Enumarate_Audio_Session(a);
		compare += s.name;
	}

	for (int a = 0; a < 36; a++) {
		if (compare.find(get(a).name) == std::string::npos && get(a).name != "UNUSED" && get(a).name != "RESERV") {
			return get(a);
		}
	}
	session se;
	se.name = "excluded";
	return se;
}

session format_sessions::get_recently_opened_program() {
	push_sessions_to_buffer();

	/*
	I add all the sessions into a single string, which I can
	then can compare against by checking if the NEW enumeration buffer contains
	a name -- if it DOESNT return it.
	*/

	std::string compare;
	for (int a = 0; a < 36; a++) {
		if(get(a).name != "UNUSED" && get(a).name != "RESERV")
			compare += get(a).name;
	}

	for (int a = 0; a < (int)buffer.size(); a++) {
		//printf("%s\n", get(a).name.c_str());
		if (compare.find(buffer[a].name) == std::string::npos && buffer[a].name != "UNUSED" && buffer[a].name != "RESERV") {
			return buffer[a];
		}
	}
	return session();
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
	return buff;
}