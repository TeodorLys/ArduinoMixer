#pragma once
#include <string>
#include <array>

class regular_sessions {
private:
	struct entity {
		std::string name = "NILL";
		long long date = 0;
		int timeout = 259200;
		entity(std::string _name, long long _date, int _timeout) : name(_name), date(_date), timeout(_timeout) {}
		entity(){}
		bool is_empty() const {
			return (name == "NILL" && date == 0);
		}
	};

	const int default_timeout = 259200; //3 days in seconds
	std::array<entity, 64> sessions;
	bool need_update = false;

	enum {
		TIMEOUT_INFINITE = -1
	};

public:
	regular_sessions();
	void add_entity(std::string name, bool to_reserv = false);
	void update_entity(std::string name, bool to_reserv = false, bool remove = false);
	bool exists(std::string name);
private:
	entity parse_entity(std::string s);
	std::string parse_csv(std::string &s, bool erase = true);
	bool has_timed_out(entity e);
	void write_file();
};

