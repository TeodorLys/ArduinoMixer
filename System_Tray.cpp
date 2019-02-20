#include "System_Tray.h"
#include "Global_Variables.h"
#include "BalloonTip.h"
#include "resource.h"
#include "Network_Functionality.h"
#include "Load_Externals.h"
#include "Arduino_Parse.h"

Network_Functionality *System_Tray::nf = nullptr;
Arduino_Parse *System_Tray::ap = nullptr;
Load_Externals *System_Tray::load = nullptr;
std::unique_ptr<Settings_GUI> *System_Tray::gui = nullptr;
NOTIFYICONDATA System_Tray::notify;

LRESULT CALLBACK System_Tray::OnEvent(HWND Handle, UINT message, WPARAM wParam, LPARAM lParam) {
	INT wmid, wmEvent;

	switch (message) {
	case WM_CLOSE:
		PostQuitMessage(0);
		Shell_NotifyIcon(NIM_DELETE, &notify);
		return 0;

	case WM_APP:
		switch (lParam) {
			//If you click the system tray icon
		case WM_RBUTTONDOWN: case WM_LBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextWindow(Handle);

		case NIN_BALLOONUSERCLICK:
			if (BalloonTip::_Get_Identifier() == 1) {
				nf->Download_Updates();
				BalloonTip::_Set_Identifier(0);
			}
			break;
		}
		break;

	case WM_COMMAND:
		wmid = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Button events for when you click the system tray icon
		switch (wmid) {
		case AM_EXIT:
			PostQuitMessage(0);
			Shell_NotifyIcon(NIM_DELETE, &notify);
			return 0;

		case AM_TOGGLE:
			ap->Arduino_Display_Toggle();
			break;

		case AM_UPDATES:
			if (nf->Check_For_Updates(false))
				load->Update_Save_File();
			break;

		case AM_WAITING:
			nf->Download_Updates();
			break;
		case AM_SETTINGS:
			(*gui) = std::make_unique<Settings_GUI>();
			(*gui)->Launch_GUI();
			break;

		}
		return 1;
	}

	return DefWindowProc(Handle, message, wParam, lParam);
}

void System_Tray::ShowContextWindow(HWND hwnd) {
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if (hMenu) {
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_SETTINGS, "SETTINGS");
		if (!global::waiting_Update)
			InsertMenu(hMenu, -1, MF_BYPOSITION, AM_UPDATES, "CHECK FOR UPDATES");
		else
			InsertMenu(hMenu, -1, MF_BYPOSITION, AM_WAITING, "INSTALL UPDATES");
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_TOGGLE, "TOGGLE DISPLAY");
		InsertMenu(hMenu, -1, MF_BYPOSITION, AM_EXIT, "EXIT");

		SetForegroundWindow(hwnd);

		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hwnd, NULL);
		DestroyMenu(hMenu);
	}
}

System_Tray::System_Tray(Network_Functionality *net, Arduino_Parse *ard, Load_Externals *ext, std::unique_ptr<Settings_GUI> *g) {
	nf = net;
	ap = ard;
	load = ext;
	gui = g;

}


System_Tray::~System_Tray(){
	Shell_NotifyIcon(NIM_DELETE, &notify);
	DestroyWindow(window);
	UnregisterClass("NON", hinst);
}

void  System_Tray::Start() {
	hinst = GetModuleHandle(NULL);
	WNDCLASS windowclass;
	windowclass.style = 0;
	windowclass.lpfnWndProc = &OnEvent;
	windowclass.cbClsExtra = 0;
	windowclass.cbWndExtra = 0;
	windowclass.hInstance = hinst;
	windowclass.hIcon = NULL;
	windowclass.hCursor = 0;
	windowclass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	windowclass.lpszMenuName = NULL;
	windowclass.lpszClassName = "NON";
	RegisterClass(&windowclass);

	HWND window = CreateWindow("non", "non", WS_SYSMENU | WS_MINIMIZE, 0, 0, 400, 500, NULL, NULL, hinst, NULL);
	Register_System_Tray_Icon(window);
}

void System_Tray::Register_System_Tray_Icon(HWND hwnd) {
	ZeroMemory(&notify, sizeof(notify));
	notify.cbSize = sizeof(NOTIFYICONDATA);

	notify.uID = 5;
	notify.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	notify.hIcon = (HICON)LoadImage(hinst, MAKEINTRESOURCE(MYICON), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

	strcpy_s(notify.szTip, tip.c_str());
	notify.uCallbackMessage = WM_APP;  // What message to look for when creating the event handler.
	notify.hWnd = hwnd;

	if (Shell_NotifyIcon(NIM_ADD, &notify) == FALSE) {
		printf("Could not add...\n");
		exit(0);
	}
}