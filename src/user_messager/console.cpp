#include "console.h"
#include "crash_logger.h"
#include <Windows.h>

bool console::console_is_active = false;

bool console::allocate_console() {
	if (AllocConsole() == 0) {
		crash_logger cl;
		cl.log_message_with_last_error("Could not allocate console", __FUNCTION__);
		FreeConsole();
		return false;
	}
	errno_t err;
	err = freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
	if (err != 0) {
		crash_logger cl;
		cl.log_message("could not create console output", __FUNCTION__);
		FreeConsole();
		return false;
	}

	err = freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console

	if (err != 0) {
		crash_logger cl;
		cl.log_message("could not create console input", __FUNCTION__);
		FreeConsole();
		return false;
	}

	AttachConsole(GetProcessId(GetStdHandle(-10)));
	/*
	Makes sure that the user does not close the console window.
	Because if the user does... the program will terminate.
	*/
	HWND hwnd = GetConsoleWindow();
	HMENU menu = GetSystemMenu(hwnd, FALSE);
	if (EnableMenuItem(menu, SC_CLOSE, MF_GRAYED) != 0) {
		crash_logger cl;
		cl.log_message_with_last_error("Could not grayout the close button on the console", __FUNCTION__);
		return false;
	}
	console_is_active = true;
	return true;
}

bool console::deallocate_console() {
	/*
	TODO: Do som actual error checking!
	Not just return true!!!
	*/
	HWND hwnd = GetConsoleWindow();
	HMENU menu = GetSystemMenu(hwnd, FALSE);
	EnableMenuItem(menu, SC_CLOSE, MF_ENABLED);
	printf("THIS WINDOW CAN NOW BE CLOSED!!!");  // This needs to be printed before the free:ing, 
												 // because it detaches the cin and cout handlers,	
												 // Which means you cant write to the console
	FreeConsole();
	DestroyWindow(hwnd);
	CloseWindow(hwnd);
	return true;
}