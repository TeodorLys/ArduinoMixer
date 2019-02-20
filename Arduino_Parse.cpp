#include "Arduino_Parse.h"
#include "Global_Variables.h"
#include <myio.h>

Arduino_Parse::Arduino_Parse() {
	tstart = std::chrono::high_resolution_clock::now();
	tend = std::chrono::high_resolution_clock::now();
}

// Format and compress the text for the arduino LCD Screen
std::string Arduino_Parse::Parse_Display_Text() {
	std::string buffer = "";
	std::string space_temp = "     ";  // Default spacing between the audio names
	for (int a = 4 * (global::page_Number - 1); a < 4 + (4 * (global::page_Number - 1)); a++) {
		buffer += global::programs[a];
		if (a != 3)
			buffer += space_temp;
	}
	if (buffer.size() != 40) {
		buffer.resize(40, (char)32);
	}
	std::string page = "PAGE " + std::to_string(global::page_Number);
	page += "/" + std::to_string(MAX_PAGE_NUMBER);
	page += "\n";
	buffer = page + buffer;
	return buffer;
}

void Arduino_Parse::Try_to_Connect() {
	if (!is_Connected) {
		if (global::com_Port.find("\\") == std::string::npos)
			global::com_Port = "\\\\.\\" + global::com_Port;  // This should really be in the library...
		if(arduino.connect(global::com_Port.c_str(), 1000000)){
			printf("Connected...\n");
			is_Connected = true;
			std::string temp = Parse_Display_Text();
			Sleep(500);  //Wait for the arduino to initialize.
			//Sends the LCD Display text
			arduino.writeTo(temp.c_str(), temp.size());
		}
	}
}

void Arduino_Parse::Update_LCD_Screen() {
	std::string temp = Parse_Display_Text();
	if (is_Connected)
		arduino.writeTo(temp.c_str(), temp.size());
	else
		printf("DEBUG MODE IS ACTIVATED!\n");
}

void Arduino_Parse::Arduino_Display_Toggle() {
	/*Sends a |((char)124) to the arduino because any other text or such
			will be parsed as a update LCD screen function*/
	std::string temp = "|";
	if (is_Connected) {
		arduino.writeTo(temp.c_str(), temp.size());
		display_Off = true;
	}
	else
		printf("DEBUG MODE IS ACTIVATED!\n");
}

bool Arduino_Parse::Is_Open() {
	return arduino.isConnected;
}

//Time based disconnect checking, if bytes recieved is 0
//for 1 second the mixer has been disconnected
bool Arduino_Parse::Recieved_NULL_for_a_Time(int t) {
	if (t == 0) {
		if (std::chrono::duration_cast<std::chrono::seconds>(tend - tstart).count() >= 1) {
			return true;
		}
	}
	else {
		tstart = std::chrono::high_resolution_clock::now();
	}
	tend = std::chrono::high_resolution_clock::now();
	return false;
}

void Arduino_Parse::Read_Arduino_Input() {
	if (!is_Connected)
		return;
	int t = arduino.recieveFrom(s2, buffersize);

	if (t == -1 || Recieved_NULL_for_a_Time(t)) {
		printf("Arduino Disconnected...\n");
		is_Connected = false;
		Try_to_Connect();
	}

	//The only acceptable byte sizes of the serial input,
	//any lower or bigger than this will, most likly, be wrong or "corrupted".
	if (t != 8 && t != 12 && t != 4 && t != 16) {
		Sleep(5);
		return;
	}

	//NO clue why this works, I only send 4 bytes from the arduino,
	//and if I do send 8 bytes this gets SUPER weird... weird right?
	if (s2[0] != s2[4] || s2[1] != s2[5] || s2[2] != s2[6] || s2[3] != s2[7]) {
		Sleep(1);
		return;
	}

	/*FORMATTING THE SERIAL INPUT FROM THE ARDUINO*/
	for (int a = 0; a < 4; a++) {
		audio[a].value = s2[a] / 100.f;

		if (audio[a].value > 1 && audio[a].value < 1.15) {
			audio[a].value = 1;
			audio[a].last_value = (int)(audio[a].value * 100.f);
		}
		else if (audio[a].value == 0.0f)
			audio[a].value = (float)(audio[a].last_value / 100.f);
		else if (audio[a].value == 0.01)
			audio[a].value = 0.f;
		else {
			if (audio[a].value > 0)
				audio[a].last_value = (int)(audio[a].value * 100.f);
		}
	}

	//DECREASES the page number.
	if (audio[0].value == -1.f || audio[1].value == -1.f || audio[2].value == -1.f || audio[3].value == -1.f) {
		if (global::page_Number > 1 && !recent_Change) {
			global::page_Number--;
			Update_LCD_Screen();
			//printf("(--)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			Sleep(10);
		}
		return;
	}
	//INCREASES the page number.
	else if (audio[0].value == -1.2f || audio[1].value == -1.2f || audio[2].value == -1.2f || audio[3].value == -1.2f) {
		if (global::page_Number < MAX_PAGE_NUMBER && !recent_Change) {
			global::page_Number++;
			Update_LCD_Screen();
			//printf("(++)v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
			recent_Change = true;
			//Sleep(10);
		}
		return;
	}
	else {
		//printf("(  )v1: %.2f, v2: %.2f, v3: %.2f, v4: %.2f, t: %i\n", audio[0].value, audio[1].value, audio[2].value, audio[3].value, t);
		recent_Change = false;
		//Sleep(10);
		return;
	}
}