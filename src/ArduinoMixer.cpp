#include "Settings_OGUI.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "device_IO.h"
#include "Load_Externals.h"
#include "resource.h"
#include "Network_Functionality.h"
#include "Global_Variables.h"
#include "BalloonTip.h"
#include "Audio_Controller.h"
#include "System_Tray.h"
#include "crash_logger.h"
#include "initialize_device.h"
#include <Log_Print.h>

#pragma comment(linker, "/SUBSYSTEM:console /ENTRY:mainCRTStartup")

const std::string version = "2.4.0";

bool auto_open_settings = false;

int main(int argc, char** argv) {

	device_IO io;
	Network_Functionality nf;
	Load_Externals load;
	Chrome_Integration ci;
	initialize_device io_init;

	FILE* new_stdout;

	Set_Log_Print_Level(0);
	
	if (argc > 1) {
		std::string arg = argv[1];
		if (arg == "-c_connect") {
			ci.Initialize_Extension_after_Install();
		}
		else if (arg == "-after_chrome_integration") {
			auto_open_settings = true;
		}
	}


	/*
	A safety feature, so you cant have two instances of the mixer...
	*/
	nf.Give_Mutex_Handle(CreateMutex(0, FALSE, "Local\\$arduino$mixer$"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		exit(0);

	nf.set_Version(version);

	bool console_Is_Active = false;
	sf::Clock clock;
	sf::Clock update_clock;
	Settings_OGUI new_gui;
	System_Tray tray(&new_gui);
	tray.Set_Version(version);
	tray.Start();

	load.Parse_Document_Path();

	BalloonTip::Give_Reference_and_Initialize(tray.Get_Notify());

	if (std::filesystem::exists(global::documents_Settings_Path + "UPDATE_SETTINGS")) {
		load.Try_Load_Externals(true); // Loading settings file and such
		load.Complete_Update_Preload_File();
		nf.Check_For_Updates(true);
		load.Complete_Update_Save_File();
		std::filesystem::remove(global::documents_Settings_Path + "UPDATE_SETTINGS");
	}
	else {
		load.Try_Load_Externals(false); // Loading settings file and such
	}

	/*
	For the enumeration of the mixer, by the device container id, which I hope is a static and "global"
	variable...
	If the end user has multiple Teensy chips connected, this makes sure that we connect to the correct device.
	*/
	if (global::container_id == "{}") {
		io_init.retry_devices();  // Connects to all found devices and does a retrieve device info call.
	}
	else {
		io_init.find_device_by_container_id();
	}

	/*
	Recently_updated is a file created by the updater program, so this program
	can remove all the temporary install files.
	*/
	if (std::filesystem::exists(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt")) {
		printf("Recently Updated\n");
		global::waiting_Update = false;
		load.Update_Save_File();
		std::string temp = std::filesystem::temp_directory_path().string();
		try {
			std::filesystem::remove(temp + "\\Updater.exe");
			std::filesystem::remove(temp + "\\curr_temp_file.txt");
			std::filesystem::remove(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt");
		}
		catch (const std::filesystem::filesystem_error& f) {
			printf("Could not delete updater files: %s\n", f.what());
			crash_logger cl;
			cl.log_message(f.what(), __FUNCTION__);
		}
	}

	//If logging is enabled
	if (load.get_Logging()) {
		if (AllocConsole() == 0) {
			crash_logger cl;
			cl.log_message_with_last_error(__FUNCTION__);
			FreeConsole();
			goto skip_console;
		}
		errno_t err;
		err = freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
		if (err != 0) {
			crash_logger cl;
			cl.log_message("could not create console output", __FUNCTION__);
			FreeConsole();
			goto skip_console;
		}

		err = freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console

		if (err != 0) {
			crash_logger cl;
			cl.log_message("could not create console input", __FUNCTION__);
			FreeConsole();
			goto skip_console;
		}

		AttachConsole(GetProcessId(GetStdHandle(-10)));
		HWND hwnd = GetConsoleWindow();
		HMENU menu = GetSystemMenu(hwnd, FALSE);
		if (EnableMenuItem(menu, SC_CLOSE, MF_GRAYED) != 0) {
			crash_logger cl;
			cl.log_message_with_last_error(__FUNCTION__);
			FreeConsole();
			goto skip_console;
		}
		console_Is_Active = true;
	}

skip_console:

	// Tells the audio contols what values to work with.
	Audio_Controller ac(&io.audio[0], &io.audio[1], &io.audio[2], &io.audio[3]);

	if (auto_open_settings) {
		new_gui.Launch_GUI();
		auto_open_settings = false;
	}

	MSG msg;
	msg.message = ~WM_QUIT;
	int frame = 0;
	sf::Clock c;

	io.Parse_Display_Text();

	//Main loop
	while (msg.message != WM_QUIT) {
		ci.check_for_keypress();
		if (!io._Is_Connected() && clock.getElapsedTime().asMilliseconds() >= 500) {
			io.Try_to_Connect();
			clock.restart();
		}

		if (global::auto_update && update_clock.getElapsedTime().asSeconds() >= 1800) {
			nf.Check_For_Updates(true);
			update_clock.restart();
		}

		if (c.getElapsedTime() >= sf::seconds(1)) {
			//printf("%i\r", frame);
			frame = 0;
			c.restart();
		}

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (io._Is_Connected()) {
				io.Read_Arduino_Input();
				/*
				this function(below) both acts as a set volume function,
				and as a check for new programs function, also if a
				program was opened or closed.
				TODO: Two separate functions maybe.
				*/
				if (!ac.Set_Volume())
					io.Update_LCD_Screen();  //Updating the text on the arduino lcd/oled(LE-v.3) screen.
			}

			if (new_gui.Get_Needs_Update()) {
				if (global::enable_logging && !console_Is_Active) {
					AllocConsole();
					FILE* new_stdout;
					freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
					freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console
					AttachConsole(GetProcessId(GetStdHandle(-10)));
					console_Is_Active = true;
					/*
					Makes sure that the user does not close the console window.
					Because if the user does... the program will terminate.
					*/
					HWND hwnd = GetConsoleWindow();
					HMENU menu = GetSystemMenu(hwnd, FALSE);
					EnableMenuItem(menu, SC_CLOSE, MF_GRAYED);
					SetForegroundWindow(new_gui.get_Handle());
				}
				else if (!global::enable_logging && console_Is_Active) {
					HWND hwnd = GetConsoleWindow();
					HMENU menu = GetSystemMenu(hwnd, FALSE);
					EnableMenuItem(menu, SC_CLOSE, MF_ENABLED);
					printf("THIS WINDOW CAN NOW BE CLOSED!!!");
					FreeConsole();
					DestroyWindow(hwnd);
					CloseWindow(hwnd);
				}
				new_gui._confirm_Update_Executed();
				load.Update_Save_File();
				ac.Rerender_Audio_Displays();
				io.Update_LCD_Screen();
			}
		}
		frame++;
	}
}