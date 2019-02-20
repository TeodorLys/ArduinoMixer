#pragma once
#include <ArduinoConnect.hpp>
#include <chrono>

struct AudioSession {
	float value = 1.f;
	int last_value = 0;
};

class Arduino_Parse {
private:
	self::Arduino_API arduino;
	const int buffersize = 16;
	char s2[16] = "";
	const int MAX_PAGE_NUMBER = 9;
	bool recent_Change = false;  // Recently changed the LCD screen
	bool is_Connected = false;
	bool display_Off = false;
	std::chrono::high_resolution_clock::time_point tstart;
	std::chrono::high_resolution_clock::time_point tend;
private:
		std::string Parse_Display_Text();
public:
	AudioSession audio[4];
	Arduino_Parse();
	void Try_to_Connect();
	bool Is_Open();
	void Read_Arduino_Input();
	void Update_LCD_Screen();
	bool Recieved_NULL_for_a_Time(int t);
	void Arduino_Display_Toggle();
	void Arduino_Display_Off();
	void Arduino_Display_ON();
	bool is_Display_Off() { return display_Off; }
	bool _Is_Connected() { return is_Connected; }
};
