#pragma once
#include <Windows.h>
#include <string>
#include "Network_Functionality.h"

class BalloonTip {
private:
	static NOTIFYICONDATA *notify;
	static int identifier;
public:
	/*
	Registers the notification to windows, sets the type and timeout of the tip.
	*/
	static void Give_Reference_and_Initialize(NOTIFYICONDATA *n);
	/*
	Creates a ballon tip and sends it over to windows notification.
	*/
	static void Call_BallonTip(std::string title, std::string text);
	/*
	If clicked it will assign a identifier.
	SEE: System tray class in onEvent function
	*/
	static int _Get_Identifier();
	static void _Set_Identifier(int id) {
		identifier = id;
	}
};

