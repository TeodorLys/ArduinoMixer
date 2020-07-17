#include "crash_logger.h"
#include <Windows.h>
#include <filesystem>
#include <chrono>
#include <time.h>

crash_logger::crash_logger()
{
}

void crash_logger::log_message_with_last_error(std::string extra, const char* func) {
	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	log_message(messageBuffer + extra, func);
}

void crash_logger::log_message(std::string s, const char* func) {
	std::string time = "[";
	std::string _s;
	for (char c : s) {
		if (c != '\n')
			_s += c;
	}

	s = _s;

	auto end = std::chrono::system_clock::now();

	std::time_t t = std::chrono::system_clock::to_time_t(end);

	time +=  std::ctime(&t);
	time.pop_back();
	time += "]";

	std::string time_final;

	for (char c : time) {
		if (c != '\n')
			time_final += c;
	}

	printf("%s, %s\n", time.c_str(), global::documents_Settings_Path.c_str());

	//std::string crash_log_name =

	file.open(global::documents_Settings_Path + "\\crashlog.txt", std::fstream::app | std::fstream::out);

	file << time + " -> " + s + " : " + func << "\n";

	file.close();
}