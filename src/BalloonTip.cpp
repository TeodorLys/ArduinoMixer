#include "BalloonTip.h"

NOTIFYICONDATA *BalloonTip::notify = nullptr;
int BalloonTip::identifier = 0;


void BalloonTip::Give_Reference_and_Initialize(NOTIFYICONDATA *n) {
	notify = n;
	notify->uFlags = NIF_INFO;
	notify->uTimeout = 1000;
	notify->dwInfoFlags = NIIF_INFO | NIIF_NOSOUND;
}

void BalloonTip::Call_BallonTip(std::string title, std::string text) {
	strcpy_s(notify->szInfo, text.c_str());
	strcpy_s(notify->szInfoTitle, title.c_str());
	Shell_NotifyIcon(NIM_MODIFY, notify);
}

int BalloonTip::_Get_Identifier() {
	return identifier;
}