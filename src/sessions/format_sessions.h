#pragma once
#include "Global_Variables.h"
#include <string>
#include <vector>
#include <audiopolicy.h>
#include "enumerate_sessions.h"
#include "crash_logger.h"

/*
This class uses a string buffer, and that is what all the "outside" functions
will interact with.

and at the end of each string (re)structure, you just call reenumerate_sessions
and it will push each program into a buffer and assign it to the right
spot by NAME.

TODO: Make it smaller, make each function have more functionallity.
this is a big mess now...
*/
struct session_reservation {
	std::string name;
	int index;
};

class format_sessions {
private:

	struct page_sessions {
		session s[4];
		int index;
	};


	page_sessions sessions[9];
	page_sessions *active;
	enumerate_sessions enum_sessions;
	std::vector<session> buffer;
public:
	/*
	Easy fix to avoidning a "Global variable". this is a global variable but not all in the same file atleast...
	*/
	static format_sessions& get() {
		static format_sessions fs;
		return fs;
	}

	void reenumerate_sessions();
	void construct_sessions();
	session get_recently_closed_program();
	session get_recently_opened_program();
	session_reservation get_reserv_session(int index);
public:

	format_sessions() {
		for (int a = 0; a < 9; a++) {
			get_page(a).index = a + 1;
		}
		active = &get_page(0);
	}

	void push_sessions_to_buffer() {
		free_all();
		int count = enum_sessions.Get_Count();

		for (int a = 0; a < count; a++) {
			session buff = enum_sessions.Enumarate_Audio_Session(a);
			//if (Check_for_Restrictions(buff.name)) {
				buff.control->AddRef();
				buffer.push_back(buff);
			//}
			enum_sessions.free_controller();
		}
	}

	void free_all() {
		for (int a = 0; a < (int)buffer.size(); a++) {
			SAFE_RELEASE(buffer[a].control);
		}
		enum_sessions.Free_All();
		buffer.clear();
	}

	void deallocate_session(int index) {
		if (index < 0)
			return;
		if (get(index).reserved)
			get(index).name = "RESERV";
		else
			get(index).name = "UNUSED";
		get(index).initial_volume = 0.0;
		get(index).mute = false;
		get(index).reserved_name = "UNUSED";
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

	std::string check_for_default_name(std::string s) {
		for (rename_session rs : global::renamed) {
			if (s.find(rs.new_name) != std::string::npos)
				return rs.default_name;
		}
		return s;
	}

	bool Check_for_Restrictions(std::string s) {
		for (std::string ex : global::to_Exclude) {
			if (s.find(ex) != std::string::npos) {
				return false;
			}
		}
		return true;
	}

	void assign_next_unused_spot(std::string name) {
		for (int a = 0; a < 36; a++) {
			if (get(a).name == "UNUSED") {
				printf("assigned %s to %i\n", name.c_str(), a);
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

	/*
	*Sets the reserv boolean in the buffer to true, by index
	*and sets the reservation name to the default name
	*/
	void reserv_spot(int index, std::string name) {
		if(get(index).name == "UNUSED")
			get(index).name = "RESERV";
		get(index).reserved = true;
		get(index).reserved_name = name;
	}

	/*
	*Sets the reserv boolean in the buffer to false, by index
	*and free the reservation name
	*/
	void free_reserv_spot(int index) {
		if (get(index).name == "RESERV")
			get(index).name = "UNUSED";
		get(index).reserved = false;
		get(index).reserved_name = "UNUSED";
	}

	bool is_reserverd(std::string name) {
		int size = get_reserv_size();
		for (int a = 0; a < size; a++)
			if (get_reserv_session(a).name == name)
				return true;
		return false;
	}

	bool compare_session(std::string name, int index) {
			if (get(index).name == name || get(index).name == check_For_New_Name(name))
				return true;
		return false;
	}

	bool compare_reserved_session(std::string name, int index) {
		if (get(index).reserved_name == name || get(index).reserved_name == check_For_New_Name(name))
			return true;
		return false;
	}

	/*
	Returns a recently enumerated session by index.
	*ie. A program that is open and are playing or going to play some audio.
	!--This does not return anything in the (programmer) made sessions buffer ! USE "get" FOR THAT--!
	*/
	session get_active_session(int index) {
		if (index < (int)buffer.size())
			return buffer[index];
		else
			return session();
	}

	session &get(int index) {
		return(sessions[(int)floor(index / 4)].s[index % 4]);
	}

	page_sessions &get_page(int index) {
		return(sessions[index]);
	}


	/*
	Returns the currently active page on the mixer
	*/
	page_sessions get_active_page() {
		return *active;
	}

	/*
	Returns the volume on a session when it was enumerated.
	*/
	float get_initial_volume(int index) {
		return get(index).initial_volume;
	}

	/*
	Goes to the next page on the mixer
	*/
	void plus_active_page_session() {
		int i = active->index - 1;
		if (i + 1 < 9) {
			//printf("going to: %i, %i\n", i, active->index);
			active = &get_page(i + 1);
		}
	}

	/*
	Goes to the previous page on the mixer
	*/
	void minus_active_page_session() {
		int i = active->index - 1;
		if (i - 1 >= 0) {
			//printf("going to: %i, %i\n", i, active->index);
			active = &get_page(i - 1);
		}
	}
	
	/*
	Sets the volume when the knobs have been turned on the mixer.
	*/
	void set_active_volume_parameters() {
		for (int a = 0; a < 4; a++) {
			if (active->s[a].volume == nullptr) {
				crash_logger cl;
				cl.log_message_with_last_error(std::string("The volume handler was nullptr"), __FUNCTION__);
				exit(0);
			}
			if (active->s[a].control == NULL)
				continue;
			active->s[a].control->SetMasterVolume(*active->s[a].volume, NULL);
			active->s[a].initial_volume = *active->s[a].volume;
		}
	}

	/*
	When checking for exclusion or reserved spots, this becomes very useful.
	*/
	bool active_in_buffer(std::string s) {
		for (int a = 0; a < (int)buffer.size(); a++) {
			if (buffer[a].name == s)
				return true;
		}
		return false;
	}

	/*
	Gets the index of a sessions name, if it exists in the sesison-buffer, if not it returns -1
	*/
	int get_index(std::string name) {
		for (int a = 0; a < 36; a++) {
			if (check_for_default_name(get(a).name) == name) {
				return a;
			}
		}
		return -1;
	}

	int recent_buffer_size() {
		return buffer.size();
	}

};
