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
	void log_message_with_last_error(const char* func);
	void log_message(std::string s, const char* func);
};

