#pragma once
#include "Global_Variables.h"
#include <string>
#include <vector>
#include <audiopolicy.h>
#include "enumerate_sessions.h"

struct session_reservation {
	std::string name;
	int index;
};

class format_sessions {
private:

	struct page_sessions {
		session s[4];
	};
	page_sessions sessions[9];
	page_sessions active;
	enumerate_sessions enum_sessions;
	std::vector<session> buffer;
public:
	static format_sessions& get() {
		static format_sessions sf;
		return sf;
	}

	void reconstruct_sessions();
	session get_recently_closed_program();
	session get_recently_opened_program();
	session_reservation get_reserv_session(int index);
public:

	void push_sessions_to_buffer() {
		free_all();
		int count = enum_sessions.Get_Count();

		for (int a = 0; a < count; a++) {
			session buff = *enum_sessions.Enumarate_Audio_Session(a);
			buff.control->AddRef(); 
			buffer.push_back(buff);
			enum_sessions.free_controller();
		}
	}

	void free_all() {
		for (int a = 0; a < buffer.size(); a++) {
			SAFE_RELEASE(buffer[a].control);
		}
		enum_sessions.Free_All();
		buffer.clear();
	}

	void deallocate_session(int index) {
		if (get(index).reserved)
			get(index).name = "RESERV";
		else
			get(index).name = "UNUSED";
		get(index).initial_volume = 0.0;
		get(index).mute = false;
		get(index).reserved_name = "";
		SAFE_RELEASE(get(index).control);
	}

	void set_volume_pointers(AudioSession *v[4]) {
		for (int a = 0; a < 9; a++) {
			get_page(a).s[0].volume = &v[0]->value;
			get_page(a).s[1].volume = &v[1]->value;
			get_page(a).s[2].volume = &v[2]->value;
			get_page(a).s[3].volume = &v[3]->value;
		}
	}

	int get_reserv_size() {
		int count = 0;
		for (int a = 0; a < 36; a++) {
			if (get(a).reserved)
				count++;
		}
		return count;
	}

	std::string check_For_New_Name(std::string s) {
		for (rename_session rs : global::renamed) {
			if (s.find(rs.default_name) != std::string::npos)
				return rs.new_name;
		}
		return s;
	}

	void assign_next_unused_spot(std::string name) {
		for (int a = 0; a < 36; a++) {
			if (get(a).name == "UNUSED") {
				printf("assigned: %i, %s\n", a, name.c_str());
				assign_session_spot(a, name);
				return;
			}
		}
	}

	void assign_session_spot(int index, std::string name) {
		get(index).name = name;
	}

	void reset_all_controllers() {
		for (int a = 0; a < 36; a++) {
			if(!get(a).reserved)
				get(a).name = "UNUSED";
			else 
				get(a).name = "RESERV";
			SAFE_RELEASE(get(a).control);
		}
	}

	void reserv_spot(int index, std::string name) {
		get(index).reserved = true;
		get(index).reserved_name = name;
	}

	bool is_reserverd(std::string name) {
		int size = get_reserv_size();
		for (int a = 0; a < size; a++)
			if (get_reserv_session(a).name == name)
				return true;
		return false;
	}

	void assign_controller(int index, ISimpleAudioVolume* v) {
		get(index).control = v;
	}

	bool compare_session(std::string name, int index) {
			if (get(index).name == name || get(index).name == check_For_New_Name(name))
				return true;
		return false;
	}

	session get_active_session(int index) {
		if (index < buffer.size())
			return buffer[index];
		else
			return session();
	}

	session &get(int index) {
		return(sessions[(int)floor(index / 4)].s[index % 4]);
	}

	page_sessions get_page(int index) {
		return(sessions[(int)floor(index / 4)]);
	}
};
