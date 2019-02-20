#include <SFML/Graphics.hpp>
#include <Windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "Arduino_Parse.h"
#include "Load_Externals.h"
#include "resource.h"
#include "Network_Functionality.h"
#include "Global_Variables.h"
#include "Settings_GUI.h"
#include "BalloonTip.h"
#include "Audio_Control.h"
#include "System_Tray.h"

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

Arduino_Parse ap;
Network_Functionality nf;
Load_Externals load(&nf);
std::unique_ptr<Settings_GUI> gui;
FILE* new_stdout;

const std::string version = "2.0.2";

int main() {
	CreateMutex(0, FALSE, "Local\\$myprogram$");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		exit(0);

	nf.set_Version(version);

	bool console_Is_Active = false;
	sf::Clock clock;

	System_Tray tray(&nf, &ap, &load, &gui);
	tray.Set_Version(version);
	tray.Start();

	load.Parse_Document_Path();

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



	BalloonTip::Give_Reference_and_Initialize(tray.Get_Notify());

	if (std::experimental::filesystem::exists(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt")) {
		printf("Recently Updated\n");
		global::waiting_Update = false;
		load.Update_Save_File();
		std::string temp = std::experimental::filesystem::temp_directory_path().string();
		std::experimental::filesystem::remove(temp + "\\Updater.exe");
		std::experimental::filesystem::remove(std::experimental::filesystem::temp_directory_path().string() + "\\curr_temp_file.txt");
		std::experimental::filesystem::remove(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt");
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
	Audio_Control ac(&ap.audio[0].value, &ap.audio[1].value, &ap.audio[2].value, &ap.audio[3].value);

	MSG msg;
	msg.message = ~WM_QUIT;
	int frame = 0;
	sf::Clock c;
	//Main loop
	while (msg.message != WM_QUIT) {
		if (!ap._Is_Connected() && clock.getElapsedTime().asMilliseconds() >= 500) {
			ap.Try_to_Connect();
			clock.restart();
		}
		else if (!ap._Is_Connected() && gui == nullptr) {
			Sleep(50);
		}
		if (c.getElapsedTime() >= sf::seconds(1)) {
			//printf("%i\r", frame);
			frame = 0;
			c.restart();
		}
		if (gui != nullptr)
			if (!gui->get_Open()) {
					Settings_GUI* ptr = gui.release();
					delete ptr;
			}

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			ap.Read_Arduino_Input();
			/*
			this function(below) both acts as a set volume function,
			and as a check for new programs function, also if it is a 
			program was opened or closed.
			*/
			if (!ac.Set_Volume()) 
				ap.Update_LCD_Screen();  //Updating the text on the arduino lcd screen.

			if (gui != nullptr) {
				if (gui->get_Open())
					gui->_Draw_and_Check_Poll();
				if (gui->Get_Needs_Update()) {
					if (global::enable_logging && !console_Is_Active) {
						AllocConsole();
						FILE* new_stdout;
						freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
						freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console
						AttachConsole(GetProcessId(GetStdHandle(-10)));
						console_Is_Active = true;
						HWND hwnd = GetConsoleWindow();
						HMENU menu = GetSystemMenu(hwnd, FALSE);
						EnableMenuItem(menu, SC_CLOSE, MF_GRAYED);
						SetForegroundWindow(gui->get_Handle());
					}
					else if (!global::enable_logging && console_Is_Active) {
						HWND hwnd = GetConsoleWindow();
						HMENU menu = GetSystemMenu(hwnd, FALSE);
						EnableMenuItem(menu, SC_CLOSE, MF_ENABLED);
						printf("THIS WINDOW CAN NOW BE CLOSED!!!");
						FreeConsole();
					}
					gui->_confirm_Update_Executed();
					global::pref = gui->Extract_Prefered_List_cont();
					global::reserved_List.clear();  //Dont know enough about vectors to understand
																					//why this is needed... maybe its not(needed, that is).
					global::reserved_List = gui->Extract_Reserved_List();
					load.Update_Save_File();
					ac.Rerender_Audio_Displays();
					ap.Update_LCD_Screen();
				}
			}
		}
		frame++;
	}
}