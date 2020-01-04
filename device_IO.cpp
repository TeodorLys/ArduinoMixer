#include "Global_Variables.h"
#include "device_IO.h"
#include "crash_logger.h"
#include <myio.h>

device_IO::device_IO() {
	tstart = std::chrono::high_resolution_clock::now();
	tend = std::chrono::high_resolution_clock::now();
}

self::Arduino_API device_IO::arduino;
bool device_IO::display_Off = false;
bool device_IO::recent_Change = false;
bool device_IO::is_Connected = false;
std::string device_IO::_com_port;

// Format and compress the text for the arduino LCD Screen
std::string device_IO::Parse_Display_Text() {
	std::string buffer = "";
	const std::string space_temp = "     ";  // Default spacing between the audio names
	std::string volume_buffer;
	Audio_Controller control;
	for (int a = 4 * (global::page_Number - 1); a < 4 + (4 * (global::page_Number - 1)); a++) {
		buffer += global::programs[a];
		int ivol = static_cast<int>(control.get_last_value_of_sliders(a) * 100);

		clamp_to_nearest(ivol, 5);
		volume_buffer += std::to_string(ivol) + ";";
		if (a != 3)
			buffer += space_temp;
	}

	if (buffer.size() != 40)
		buffer.resize(40, (char)32);
	
	/*
	Versions 4 and up uses encoders for the volume controller,
	which means you can set the starting volume to anything you want!
	with 3 and down, potentiometers were used.
	*/
	if (global::version >= 4)
		buffer += ";" + volume_buffer;

	//printf("%s\n", buffer.c_str());

	return buffer;
}

void device_IO::Try_to_Connect() {
	if (!is_Connected) {
		if (_com_port.find("\\") == std::string::npos)
			_com_port = "\\\\.\\" + _com_port;  // This should really be in the library...
		if(arduino.connect(_com_port.c_str(), 1000000)) {
			printf("Connected...\n");
			is_Connected = true;
			Sleep(500);  //Wait for the arduino to initialize.
			//Sends the LCD Display text
			Get_Mixer_Version();
			std::string temp = Parse_Display_Text();
			arduino.writeTo(temp.c_str(), temp.size());
		}
	}
}

void device_IO::Get_Mixer_Version() {
	arduino.writeTo("y", 1);
	Sleep(10);
	char buffer[] = "0";
	int t = arduino.recieveFrom(buffer, 1);
	global::version = (int)buffer[0] - 48;
}

void device_IO::Update_LCD_Screen() {
	std::string temp = Parse_Display_Text();
	if (is_Connected)
		arduino.writeTo(temp.c_str(), temp.size());
	else
		printf("ULS DEBUG MODE IS ACTIVATED!\n");
}

void device_IO::Arduino_Display_Toggle() {
	/*Sends a "|"((char)124) to the arduino because any other text or such
			will be parsed as a update LCD screen function*/
	std::string temp = "|";
	if (is_Connected) {
		arduino.writeTo(temp.c_str(), temp.size());
		display_Off = true;
	}
	else
		printf("ADT DEBUG MODE IS ACTIVATED!\n");
}

void device_IO::Toggle_Volume_Procentage() {
	/*Sends a "{"((char)123) to the arduino because any other text or such
		will be parsed as a update LCD screen function*/
	std::string temp = "{";
	if (is_Connected) {
		arduino.writeTo(temp.c_str(), temp.size());
	}
	else
		printf("TVP DEBUG MODE IS ACTIVATED!\n");
}

void device_IO::Dehook_from_mixer() {
	/*Sends a "}"((char)125) to the arduino because any other text or such
		will be parsed as a update LCD screen function*/
	std::string temp = "}";
	if (is_Connected) {
		arduino.writeTo(temp.c_str(), temp.size());
		is_Connected = false;
		global::prog_ending = true;
	}
	else
		printf("DFM DEBUG MODE IS ACTIVATED!\n");
}

bool device_IO::Is_Open() {
	return arduino.isConnected;
}

//Time based disconnect checking, if bytes recieved is 0
//for 1 second the mixer has been disconnected
/*Quite obsolete at this time...*/
bool device_IO::Recieved_NULL_for_a_Time(int t) {
	if (t == 0) {
		if (std::chrono::duration_cast<std::chrono::seconds>(tend - tstart).count() >= 5) {
			return true;
		}
	}
	else {
		tstart = std::chrono::high_resolution_clock::now();
	}
	tend = std::chrono::high_resolution_clock::now();
	return false;
}

void device_IO::check_for_disconnection() {
	std::string temp = "z";
	if (is_Connected) {
		if (!arduino.writeTo(temp.c_str(), temp.size())) {
			printf("disconnected!\n");
			is_Connected = false;
			Try_to_Connect();
		}
	}
	else
		printf("CFD DEBUG MODE IS ACTIVATED!\n");
}

void device_IO::Read_Arduino_Input() {
	if (!is_Connected)
		return;
	int t = arduino.recieveFrom(s2, buffersize);
	if (t < 2) {
		for (int a = 0; a < 4; a++) {
			audio[a].has_changed = false;
		}
		Sleep(10);
		return;
	}
	else {
		for (int a = 0; a < 4; a++) {
			audio[a].has_changed = true;
		}
	}

	//The only acceptable byte sizes of the serial input,
	//any lower or bigger than this will, most likly, be wrong or "corrupted".
	if (t != 4) {
		Sleep(5);
		return;
	}
	//printf("%i, %i, %i, %i\n", s2[0], s2[1], s2[2], s2[3]);
	/*
	Formatting and filtering the input from the mixer!
	*/
	for (int a = 0; a < 4; a++) {
		if (s2[a] == 0) {
			audio[a].value = 0.f;
			audio[a].last_value = (int)(audio[a].value * 100.f);
		}
		else {
			audio[a].value = s2[a] / 100.f;

			if (audio[a].value > 1.0 && audio[a].value < 1.15) {
				audio[a].value = 1;
				audio[a].last_value = (int)(audio[a].value * 100.f);
			}
			else if (audio[a].value == 0.0f)
				audio[a].value = (float)(audio[a].last_value / 100.f);
			else if (audio[a].value == 0.01)
				audio[a].value = 0.f;
			else {
				if (audio[a].value >= 0.0)
					audio[a].last_value = (int)(audio[a].value * 100.f);
			}
		}
	}

	//printf("%.2f, %.2f, %.2f, %.2f\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value);

	//DECREASES the page number.
	if (audio[0].value == -1.f || audio[1].value == -1.f || audio[2].value == -1.f || audio[3].value == -1.f) {
		if (global::page_Number > 1 /*&& !recent_Change*/) {
			global::page_Number--;
			Update_LCD_Screen();
			printf("(--)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			//recent_Change = true;
			Sleep(100);
		}
		return;
	}
	//INCREASES the page number.
	else if (audio[0].value == -1.2f || audio[1].value == -1.2f || audio[2].value == -1.2f || audio[3].value == -1.2f) {
		if (global::page_Number < MAX_PAGE_NUMBER /*&& !recent_Change*/) {
			global::page_Number++;
			Update_LCD_Screen();
			printf("(++)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			Sleep(100);
		}
		return;
	}
	else if (audio[0].value == -0.9f || audio[1].value == -0.9f || audio[2].value == -0.9f || audio[3].value == -0.9f) {
		printf("(SW1)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
		recent_Change = true;
		Sleep(100);
		return;
	}
	else if (audio[0].value == -0.8f || audio[1].value == -0.8f || audio[2].value == -0.8f || audio[3].value == -0.8f) {
			printf("(SW2)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			Sleep(100);
		return;
	}
	else if (audio[0].value == -0.7f || audio[1].value == -0.7f || audio[2].value == -0.7f || audio[3].value == -0.7f) {
			printf("(SW3)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			Sleep(100);
		return;
	}
	else if (audio[0].value == -0.6f || audio[1].value == -0.6f || audio[2].value == -0.6f || audio[3].value == -0.6f) {
			printf("(SW4)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			Sleep(100);
		return;
	}
	else {
		if(audio[0].has_changed)
		printf("(  )v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
		recent_Change = false;
		//Sleep(10);
		return;
	}
}