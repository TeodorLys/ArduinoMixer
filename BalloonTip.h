#pragma once
#include <Windows.h>
#include <string>
#include "Network_Functionality.h"

class BalloonTip {
private:
	static NOTIFYICONDATA *notify;
	static int identifier;
public:
	static void Give_Reference_and_Initialize(NOTIFYICONDATA *n);
	static void Call_BallonTip(std::string title, std::string text);
	static int _Get_Identifier();
	static void _Set_Identifier(int id) {
		identifier = id;
	}
};

