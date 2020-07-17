#include "Global_Variables.h"
#include <Windows.h>
#include <string>
#include <iostream>
#include <filesystem>
#include "device_IO.h"
#include "settings_handler.h"
#include "resource.h"
#include "network_handler.h"
#include "BalloonTip.h"
#include "Audio_Controller.h"
#include "System_Tray.h"
#include "crash_logger.h"
#include "initialize_device.h"
#include "Settings_WPF.h"
#include "console.h"
#include "update_handler.h"
#include <Log_Print.h>

#pragma comment(linker, "/SUBSYSTEM:console /ENTRY:mainCRTStartup")

bool auto_open_settings = false;

int main(int argc, char** argv) {

	device_IO io;
	network_handler nf;
	update_handler _update;
	settings_handler load;
	Chrome_Integration ci;
	initialize_device io_init;
	console _console;
	FILE* new_stdout;

	Set_Log_Print_Level(0);
	/*
	A safety feature, so you cant have two instances of the mixer...
	*/
	nf.Give_Mutex_Handle(CreateMutex(0, FALSE, "Local\\$arduino$mixer$"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		exit(0);

	sf::Clock clock;
	sf::Clock update_clock;
	Settings_WPF wpf;
	load.Try_settings_handler(false); // Loading settings file and such

	if (argc > 1) {
		std::string arg = argv[1];
		if (arg == "-c_connect") {
			ci.Initialize_Extension_after_Install();
		}
		else if (arg == "-after_chrome_integration") {
			auto_open_settings = true;
		}
		else if (arg == "-UPDATED") {
			/*
			Recently_updated is a file created by the updater program, so this program
			can remove all the temporary install files.
			*/
			_update.finish_and_clean_install();
		}
	}

	System_Tray tray(&wpf);
	tray.Start();
	BalloonTip::Give_Reference_and_Initialize(tray.Get_Notify());


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

	//If logging is enabled
	if (load.get_Logging()) {
		_console.allocate_console();
	}

	// Tells the audio contols what values to work with.
	Audio_Controller ac(&io.audio[0], &io.audio[1], &io.audio[2], &io.audio[3]);

	if (auto_open_settings) {
		wpf.launch();
		auto_open_settings = false;
	}

	MSG msg;
	msg.message = ~WM_QUIT;
	int frame = 0;
	sf::Clock c;

	io.Parse_Display_Text();  // The initial communication with the mixer

	/*
	This windows messaging loop is for the tray icon, and also event listeners.
	Like checking for a device change etc.
	*/
	while (msg.message != WM_QUIT) {
		/*
		TODO: MAYBE @DEPRECATED!
		The current version of chrome has a built in play/pause function, 
		it uses the media play pause button, maybe remove the extension creator class
		and just modify the check for keypress functionm?
		*/
		ci.check_for_keypress();
		if (!io._Is_Connected() && clock.getElapsedTime().asMilliseconds() >= 500) {
			io.Try_to_Connect();
			clock.restart();
		}

		if (global::auto_update && update_clock.getElapsedTime().asSeconds() >= 1800) {
			_update.check_for_updates(true);
			update_clock.restart();
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
					io.Update_LCD_Screen();  //Updating the text on the arduino lcd/oled(LE-v.3-1) screen.
			}

			if (wpf.get_needs_update()) {
				load.reread_all_data();
				if (global::enable_logging && !_console.console_active()) {
					_console.allocate_console();
				}
				else if (!global::enable_logging && _console.console_active()) {
					_console.deallocate_console();
				}
				wpf.confirm_update();
				ac.Rerender_Audio_Displays();
				io.Update_LCD_Screen();
			}
		}
	}
}