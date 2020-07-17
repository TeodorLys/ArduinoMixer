#pragma once
#include "Global_Variables.h"
#include <ArduinoConnect.hpp>
#include <chrono>
#include "Audio_Controller.h"

//Serial_Input, serial_handler, device_handler, device_IO
class device_IO {
private:
	const std::string CALL_AUTHENTICATION = "~"; // (CHAR) 126 @deprecated REPLACED BY CALL_INFO_COMMAND
	const std::string CALL_TOGGLE_SCREEN = "|"; // (CHAR) 125
	const std::string CALL_TOGGLE_VOLUME = "{"; // (CHAR) 124
	const std::string CALL_DRIVER_OFF = "}"; // (CHAR)123, TELLS THE MIXER TO ENTER SLEEP MODE
	const std::string CALL_CHECK_COMMAND = "z"; // (CHAR)122 JUST A SANITY CHECK, IF THE DEVICE IS STILL CONNECTED
	const std::string CALL_VERSION_COMMAND = "y"; // (CHAR)121
	const std::string CALL_INFO_COMMAND = "x"; // (CHAR) 120

	struct device_info {
		std::string hardware_version;
		int i_hardware_version;
		std::string software_version;
		std::string creator;
	};

	static self::Arduino_API arduino;
	const int buffersize = 16;
	char s2[16] = "";
	const int MAX_PAGE_NUMBER = 9;
	static bool recent_Change;  // Recently changed the LCD screen
	static bool is_Connected;
	static bool display_Off;
	static std::string _com_port;
	device_info info_packet;
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
//private:
public:
		std::string Parse_Display_Text();
public:
	AudioSession audio[4];
	device_IO();
	/*
	Will try to connect to the saved serial port, in "_com_port",
	MAYBE_TODO: move this into the initialize_device class.
	*/
	void Try_to_Connect();
	bool Is_Open();
	/*
	Reads the mixers output and parses it into a readable mess... 
	which then the audio_controller class will read and execute the change.
	TODO: change the name of this to ex. Read_Mixer_Input?
	*/
	void Read_Arduino_Input();

	/*
	Realtime settings changes on the mixer!
	*/
	void Arduino_Display_Toggle();
	void Toggle_Volume_Procentage();
	void Dehook_from_mixer();
	void Update_LCD_Screen();
	void Get_Mixer_Version();
	/*
	@DEPRECATED 
	The mixer no longer send a continous stream of data!
	-------------------
	Will still read mixer output, but the amount of bytes is 0
	for 1sec it will disconnect and try again.
	*/
	bool Recieved_NULL_for_a_Time();
	/*
	System_Tray class will call on this function, if it retrieves a WM_DEVICECHANGE event,
	to see if the mixer was disconnected.
	*/
	void check_for_disconnection();
	bool is_Display_Off() { return display_Off; }
	bool _Is_Connected() { return is_Connected; }
public:

	device_info get_device_info() { return info_packet; }

	static std::string& com_port() {
		return _com_port;
	}

	void clamp_to_nearest(int &value, int nearest) {
		if (value > 100) {
			value = 100;
			return;
		}
		int ibuf = (int)(value / 10.f);
		ibuf = value - (ibuf * 10);
		value -= ibuf;

		if (ibuf < 3)
			ibuf = 0;
		else if (ibuf >= 3 && ibuf <= 5)
			ibuf = 5;
		else if (ibuf > 7)
			ibuf = 10;
		else if (ibuf <= 7)
			ibuf = 5;

		value += ibuf;

		if (value > 100)
			value = 100;
	}
};

