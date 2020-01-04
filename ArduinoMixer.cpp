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
#include <Log_Print.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

const std::string version = "2.4.0";

bool auto_open_settings = false;

int main(int argc, char** argv) {

	device_IO io;
	Network_Functionality nf;
	Load_Externals load;
	Chrome_Integration ci;
	FILE* new_stdout;

	Set_Log_Print_Level(0);
	
	if (argc > 1) {
		std::string arg = argv[1];
		if (arg == "-c_connect") {
			ci.Register_Native_Messaging_Host();
			ci.Reopen_Elevated("ArduinoMixer.exe", "-after_chrome_integration", false);

			if (AllocConsole() == 0) {
				crash_logger cl;
				cl.log_message_with_last_error(__FUNCTION__);
				exit(0);
			}
			errno_t err;
			err = freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
			if (err != 0) {
				crash_logger cl;
				cl.log_message("could not create console out", __FUNCTION__);
				exit(0);
			}

			err = freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console

			if (err != 0) {
				crash_logger cl;
				cl.log_message("could not create console in", __FUNCTION__);
				exit(0);
			}

			if (AttachConsole(GetProcessId(GetStdHandle(-10))) == 0) {
				crash_logger cl;
				cl.log_message_with_last_error(__FUNCTION__);
				exit(0);
			}

			std::string	explorer = std::filesystem::current_path().string() + "\\am_c_integration";

			ShellExecute(0, "open", "explorer", explorer.c_str(), NULL, SW_SHOW);

			printf("----------------------------------------------------------------------\n");
			printf("Go to extensions within chrome\n");
			printf("Click on \"add uncompressed extension\"\n");
			printf("and choose the extension folder within the am_c_integration folder\n");
			printf("then copy the id ex.\"gighmmpiobklfepjocnamgkkbiglidom\"\n");
			printf("and paste it here\n");
			printf("p.s. dont forget to restart the extension\n");
			printf("----------------------------------------------------------------------\n");
		_tryagain:
			printf("> ");
			std::string answer;
			std::getline(std::cin, answer);

			const std::string compare = "abcdefghijklmnopqrstuvwxyz";
			if (answer == "") {
				goto _tryagain;
			}
			for (char c : answer) {
				if (compare.find(c) == std::string::npos) {
					printf("could not parse id, please recopy it and paste it, dont include \"id:\"\n");
					goto _tryagain;
				}
			}


			ci.NMA_Manifest_Creation(answer);

			exit(0);
		}
		else if (arg == "-after_chrome_integration") {
			auto_open_settings = true;
		}
	}

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
			cl.log_message("could not create console out", __FUNCTION__);
			FreeConsole();
			goto skip_console;
		}

		err = freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console

		if (err != 0) {
			crash_logger cl;
			cl.log_message("could not create console in", __FUNCTION__);
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
	self::Arduino_API api;

	io.Parse_Display_Text();

	//Main loop
	while (msg.message != WM_QUIT) {
		ci.check_for_keypress();
		if (!io._Is_Connected() && clock.getElapsedTime().asMilliseconds() >= 500) {
			io.Try_to_Connect();
			clock.restart();
		}
		else if (!io._Is_Connected() && !global::prog_ending) {
			std::string temp = api.Get_Specific_Arduino_Port(1000000, "AM", "~");
			if (device_IO::com_port() != temp && temp != "") {
				device_IO::com_port() = temp;
				load.Update_Save_File();
			}
			Sleep(50);
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