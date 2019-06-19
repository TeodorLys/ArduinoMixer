#include "Settings_OGUI.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "Arduino_Parse.h"
#include "Load_Externals.h"
#include "resource.h"
#include "Network_Functionality.h"
#include "Global_Variables.h"
#include "BalloonTip.h"
#include "Audio_Control.h"
#include "System_Tray.h"
#include <Log_Print.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

Arduino_Parse ap;
Network_Functionality nf;
Load_Externals load(&nf);
FILE* new_stdout;

const std::string version = "2.1.5";

int main() {

	Set_Log_Print_Level(0);

	nf.Give_Mutex_Handle(CreateMutex(0, FALSE, "Local\\$arduino$mixer$"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		exit(0);

	nf.set_Version(version);

	bool console_Is_Active = false;
	sf::Clock clock;
	Settings_OGUI new_gui;
	System_Tray tray(&nf, &ap, &load, &new_gui);
	tray.Set_Version(version);
	tray.Start();

	load.Parse_Document_Path();

	BalloonTip::Give_Reference_and_Initialize(tray.Get_Notify());

	if (std::experimental::filesystem::exists(global::documents_Settings_Path + "UPDATE_SETTINGS")) {
		load.Try_Load_Externals(true); // Loading settings file and such
		load.Complete_Update_Preload_File();
		nf.Check_For_Updates(true);
		load.Complete_Update_Save_File();
		std::experimental::filesystem::remove(global::documents_Settings_Path + "UPDATE_SETTINGS");
	}
	else {
		load.Try_Load_Externals(false); // Loading settings file and such
	}

	if (std::experimental::filesystem::exists(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt")) {
		printf("Recently Updated\n");
		global::waiting_Update = false;
		load.Update_Save_File();
		std::string temp = std::experimental::filesystem::temp_directory_path().string();
		try {
			std::experimental::filesystem::remove(temp + "\\Updater.exe");
			std::experimental::filesystem::remove(temp + "\\curr_temp_file.txt");
			std::experimental::filesystem::remove(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt");
		}
		catch (const std::experimental::filesystem::filesystem_error& f) {
			printf("Could not delete updater files: %s\n", f.what());
		}
	}

		//If logging is enabled
	if (load.get_Logging()) {
		AllocConsole();
		freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
		freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console
		AttachConsole(GetProcessId(GetStdHandle(-10)));
		HWND hwnd = GetConsoleWindow();
		HMENU menu = GetSystemMenu(hwnd, FALSE);
		EnableMenuItem(menu, SC_CLOSE, MF_GRAYED);
		console_Is_Active = true;
	}

	// Tells the audio contols what values to work with.
	Audio_Control ac(&ap.audio[0], &ap.audio[1], &ap.audio[2], &ap.audio[3]);

	MSG msg;
	msg.message = ~WM_QUIT;
	int frame = 0;
	sf::Clock c;
	self::Arduino_API api;
	//Main loop
	while (msg.message != WM_QUIT) {
		if (!ap._Is_Connected() && clock.getElapsedTime().asMilliseconds() >= 500) {
			ap.Try_to_Connect();
			clock.restart();
		}
		else if (!ap._Is_Connected()) {
			std::string temp = api.Get_Specific_Arduino_Port(1000000, "AM", "~");
			if (global::com_Port != temp && temp != "") {
				global::com_Port = temp;
				load.Update_Save_File();
			}
			Sleep(50);
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
			if (ap._Is_Connected()) {
				ap.Read_Arduino_Input();
				/*
				this function(below) both acts as a set volume function,
				and as a check for new programs function, also if a
				program was opened or closed.
				*/
				if (!ac.Set_Volume())
					ap.Update_LCD_Screen();  //Updating the text on the arduino lcd/oled(LE-v.3) screen.
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
					ap.Update_LCD_Screen();
				}
			}
		frame++;
	}
}