#pragma once
#include <stdio.h>

class console {
private:
	static bool console_is_active;
	FILE* new_stdout;
public:
	bool allocate_console();
	bool deallocate_console();

public:
	static bool console_active() {
		return console_is_active;
	}
};

