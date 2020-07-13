#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Global_Variables.h"

class crash_logger {
private:
	std::fstream file;
public:
	crash_logger();
	/*
	Logs windows last error message to a crashlog within the documents data map.
	*/
	void log_message_with_last_error(const char* func);
	/*
	Logs a message to a crashlog within the documents data map.
	*/
	void log_message(std::string s, const char* func);
};

