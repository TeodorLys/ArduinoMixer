#include "regular_sessions.h"
#include "Global_Variables.h"
#include "settings_handler.h"
#include "crash_logger.h"
#include <fstream>
#include <algorithm>
#include <string>

regular_sessions::regular_sessions() {
	std::string friendly_path = settings_handler::get_documents_path() + "\\ArduinoMixerData\\friendly.txt";
	std::fstream file(friendly_path, std::fstream::in);

	bool found = false;

	for (std::string _s; std::getline(file, _s);) {
		if (_s.find("{") != std::string::npos) {
			found = true;
			continue;
		}

		if (_s.find("}") != std::string::npos)
			break;

		if (found) {
			entity buffer = parse_entity(_s);
			if (buffer.is_empty()) {
				continue;
			}
			for (int a = 0; a < sessions.size(); a++) {
				if (sessions[a].is_empty()) {
					if (!has_timed_out(buffer))
						sessions[a] = buffer;
					else {
						printf("%s has timed out!", buffer.name.c_str());
						need_update = true;
					}
					break;
				}
			}
		}
	}
	if (need_update)
		write_file();
}

void regular_sessions::add_entity(std::string name, bool to_reserv) {
	long long _t = std::time(0);

	for (int a = 0; a < sessions.size(); a++) {
		if (sessions[a].is_empty()) {
			sessions[a].name = name;
			sessions[a].date = _t;
			sessions[a].timeout = to_reserv ? TIMEOUT_INFINITE : default_timeout;
			break;
		}
	}
	write_file();
}

void regular_sessions::update_entity(std::string name, bool to_reserv, bool remove) {
	for (int a = 0; a < sessions.size(); a++) {
		if (sessions[a].name == name) {
			if (remove) {
				sessions[a] = entity();
				break;
			}
			sessions[a].name = name;
			if (sessions[a].timeout / default_timeout == 3 || to_reserv)
				sessions[a].timeout = TIMEOUT_INFINITE;
			else
				sessions[a].timeout = default_timeout * (sessions[a].timeout / default_timeout);
			break;
		}
	}
	write_file();
}

bool regular_sessions::exists(std::string name) {
	for (entity e : sessions) {
		if (!e.is_empty()) {
			if (e.name == name)
				return true;
		}
	}
	return false;
}

/* PRIVATE */
regular_sessions::entity regular_sessions::parse_entity(std::string s) {

	entity buffer;

	try {
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
		buffer.name = parse_csv(s);
		buffer.date = std::stoll(parse_csv(s));
		buffer.timeout = std::stoi(parse_csv(s));
	}
	catch (std::exception& e) {
		crash_logger cl;
		cl.log_message(e.what(), __FUNCTION__);
		return entity();
	}
	return buffer;
}

std::string regular_sessions::parse_csv(std::string &s, bool erase) {
	std::string buffer;
	for (int a = 0; a < s.size(); a++) {
		if (s[a] != ',')
			buffer += s[a];
		else {
			try {
				s.erase(s.begin(), s.begin() + (a + 1));
			}
			catch (std::exception& e) {
				crash_logger cl;
				cl.log_message(e.what(), __FUNCTION__);
				return buffer;
			}
			break;
		}
	}
	return buffer;
}

bool regular_sessions::has_timed_out(entity e) {
	if (e.timeout == TIMEOUT_INFINITE)
		return false;
	long long __t = std::time(0);

	long long date = e.date + e.timeout;

	return (__t > date);
}

void regular_sessions::write_file() {
	std::string friendly_path = settings_handler::get_documents_path() + "\\ArduinoMixerData\\friendly.txt";
	std::fstream file(friendly_path, std::fstream::out);
	file << "found_programs{\n";
	for (int a = 0; a < sessions.size(); a++) {
		if(!sessions[a].is_empty())
			file << sessions[a].name + ", " + std::to_string(sessions[a].date) + ", " + std::to_string(sessions[a].timeout) + "\n";
	}
	file << "}\n";
}