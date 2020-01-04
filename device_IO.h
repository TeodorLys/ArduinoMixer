#pragma once
#include "Global_Variables.h"
#include <ArduinoConnect.hpp>
#include <chrono>
#include "Audio_Controller.h"

//Serial_Input, serial_handler, device_handler, device_IO
class device_IO {
private:
	static self::Arduino_API arduino;
	const int buffersize = 16;
	char s2[16] = "";
	const int MAX_PAGE_NUMBER = 9;
	static bool recent_Change;  // Recently changed the LCD screen
	static bool is_Connected;
	static bool display_Off;
	static std::string _com_port;
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
//private:
public:
		std::string Parse_Display_Text();
public:
	AudioSession audio[4];
	device_IO();
	void Try_to_Connect();
	bool Is_Open();
	void Read_Arduino_Input();
	void Update_LCD_Screen();
	void Get_Mixer_Version();
	bool Recieved_NULL_for_a_Time(int t);
	void Arduino_Display_Toggle();
	void Toggle_Volume_Procentage();
	void Dehook_from_mixer();
	void check_for_disconnection();
	bool is_Display_Off() { return display_Off; }
	bool _Is_Connected() { return is_Connected; }
public:

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

