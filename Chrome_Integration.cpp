#include "Chrome_Integration.h"
#include "BalloonTip.h"
#include "Network_Functionality.h"
#include "Global_Variables.h"
#include "Zip_File_Handler.h"
#include "System_Tray.h"
#include "crash_logger.h"
#include <filesystem>
#include <fstream>
#include <iostream>

Chrome_Integration::Chrome_Integration() {
	packet << "pp";
	address = sf::IpAddress::getLocalAddress();
	port = 53000;
}

BOOL Is_Elevated() {
	BOOL fRet = FALSE;
	HANDLE hToken = NULL;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
		TOKEN_ELEVATION Elevation;
		DWORD cbSize = sizeof(TOKEN_ELEVATION);
		if (GetTokenInformation(hToken, TokenElevation, &Elevation, sizeof(Elevation), &cbSize)) {
			fRet = Elevation.TokenIsElevated;
		}
	}
	if (hToken) {
		CloseHandle(hToken);
	}
	return fRet;
}

void Chrome_Integration::Reopen_Elevated(std::string program, std::string params, bool asElevated) {
	if(asElevated)
		ShellExecute(NULL, "runas", program.c_str(), params.c_str(), 0, SW_SHOWNORMAL);
	else
		ShellExecute(NULL, NULL, program.c_str(), params.c_str(), 0, SW_SHOWNORMAL);
}

void Chrome_Integration::Register_Native_Messaging_Host() {
	HKEY hKey = 0;

	if (RegCreateKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Google\\Chrome\\NativeMessagingHosts\\com.dolby.native_messaging_host", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, NULL) == ERROR_SUCCESS) {
		std::string executable = std::filesystem::current_path().string() + "\\am_c_integration\\manifest.json";
		if (RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE*)executable.c_str(), executable.size()) != ERROR_SUCCESS) {
			MessageBox(NULL, "COULD NOT CONNECT TO CHROME", "CHROME ERROR", MB_OK);
			exit(-1);
		}
	}
	else {
		MessageBox(NULL, "COULD NOT CONNECT TO CHROME", "CHROME ERROR", MB_OK);
		exit(-1);
	}
}

void Chrome_Integration::NMA_Manifest_Creation(std::string id) {
	std::string p_temp = std::filesystem::current_path().string() + "\\am_c_integration\\am_c_integration.exe";
	std::string p2_temp;
	for (char c : p_temp) {
		p2_temp += c;
		if (c == '\\') {
			p2_temp += '\\';
		}
	}

	std::string manifest =
		"{\n" \
		"\"name\": \"com.dolby.native_messaging_host\",\n" \
		"\"description\": \"Native messaging host\",\n" \
		"\"path\": \"" + p2_temp + "\",\n" \
		"\"type\": \"stdio\",\n" \
		"\"allowed_origins\": [\n" \
		"				\"chrome-extension://" + id + "/\"\n" \
		"				]\n" \
		"}\n";

	std::string current_p = std::filesystem::current_path().string();
	std::fstream file(current_p + "\\am_c_integration\\manifest.json", std::fstream::out);
	file << manifest;
	file.close();
}

void Chrome_Integration::Initialize_Extension_after_Install() {
	FILE* new_stdout;
	Register_Native_Messaging_Host();
	Reopen_Elevated("ArduinoMixer.exe", "-after_chrome_integration", false);

	if (AllocConsole() == 0) {
		crash_logger cl;
		cl.log_message_with_last_error(__FUNCTION__);
		exit(0);
	}
	errno_t err;
	err = freopen_s(&new_stdout, "CONOUT$", "w", stdout); // Output for console
	if (err != 0) {
		crash_logger cl;
		cl.log_message("could not create console out", __FUNCTION__);
		exit(0);
	}

	err = freopen_s(&new_stdout, "CONIN$", "r", stdin);  // Input for console

	if (err != 0) {
		crash_logger cl;
		cl.log_message("could not create console in", __FUNCTION__);
		exit(0);
	}

	if (AttachConsole(GetProcessId(GetStdHandle(-10))) == 0) {
		crash_logger cl;
		cl.log_message_with_last_error(__FUNCTION__);
		exit(0);
	}

	std::string	explorer = std::filesystem::current_path().string() + "\\am_c_integration";

	ShellExecute(0, "open", "explorer", explorer.c_str(), NULL, SW_SHOW);

	printf("----------------------------------------------------------------------\n");
	printf("Go to extensions within chrome\n");
	printf("Click on \"add uncompressed extension\"\n");
	printf("and choose the extension folder within the am_c_integration folder\n");
	printf("then copy the id ex.\"gighmmpiobklfepjocnamgkkbiglidom\"\n");
	printf("and paste it here\n");
	printf("p.s. dont forget to restart the extension\n");
	printf("----------------------------------------------------------------------\n");
_tryagain:
	printf("> ");
	std::string answer;
	std::getline(std::cin, answer);

	const std::string compare = "abcdefghijklmnopqrstuvwxyz";
	if (answer == "") {
		goto _tryagain;
	}
	for (char c : answer) {
		if (compare.find(c) == std::string::npos) {
			printf("could not parse id, please recopy it and paste it, dont include \"id:\"\n");
			goto _tryagain;
		}
	}
	NMA_Manifest_Creation(answer);
	exit(0);
}

void Chrome_Integration::Manifest_Creation() {
	std::string extension_manifest =
		"{\n" \
		"\"name\": \"Arduino Mixer Extension\",\n" \
		"\"description\": \"Native messaging host, for ArduinoMixer\",\n" \
		"\"permissions\": [\n"
		"									\"nativeMessaging\",\n"
		"									\"tabs\",\n"
		"									\"activeTab\",\n"
		"									\"background\",\n"
		"									\"http://*/\", \"https://*/\"\n"
		"									],\n"
		"\"background\": {\n"
		"		\"scripts\": [\"control.js\"]\n"
		"},\n"
		"\"icons\": { \"256\": \"icon256.png\"},\n"
		"\"version\": \"1.0\",\n"
		"\"minimum_chrome_version\" : \"29\",\n"
		"\"manifest_version\" : 2\n"
		"}\n";

	std::string extension_js_control = 
	"var port = chrome.runtime.connectNative('com.dolby.native_messaging_host');\n"
	"var last_tab = 0;\n"
	"\n"
  "\n"
	"port.onMessage.addListener(function(msg) {\n"
	"	if (msg.text == \"pp\") {\n"
	"	var u1 = 0;\n"
	"		chrome.tabs.query({ active: true, url : '*://*.youtube.com/watch*', }, function(tabs){\n"
	"			u1 = tabs[0].url;\n"
	"			last_tab = tabs\n"
	"				chrome.tabs.executeScript(tabs[0].id, { file: \"playback.js\" }, function(response) {console.log(response)});\n"
	"			console.log(u1);\n"
	"		});\n"
	"	 var u = String(u1)\n"
	"  if(u.indexOf(\"youtube\") <= 0) {\n"
	"		chrome.tabs.query({ active: true, url : '*://*.netflix.com/watch*', }, function(tabs){\n"
	"			var URL = tabs[0].url;\n"
	"			last_tab = tabs\n"
	"				chrome.tabs.executeScript(tabs[0].id, { file: \"playback.js\" }, function(response) {console.log(response)});\n"
	"			console.log(URL);\n"
	"		});\n"
	"	 }\n"
	"	}\n"
	"	console.log(msg.text);\n"
	"});\n"
	"\n"
	"port.onDisconnect.addListener(function() {\n"
	"	console.log(\"Disconnected\");\n"
	"});\n";

	std::string extension_js_playback =
	"(() =>{\n"
	"	const video = document.querySelector('video');\n"
	"	if (video.paused)\n"
	"		video.play();\n"
	"	else\n"
	"		video.pause();\n"
	"	return video.paused;\n"
	"})();\n";


	try {
		std::filesystem::create_directory(std::filesystem::current_path().string() + "\\am_c_integration");
		std::filesystem::create_directory(std::filesystem::current_path().string() + "\\am_c_integration\\extension");
	}
	catch (std::exception &e) {
		printf("%s\n", e.what());
	}
	std::string current_p = std::filesystem::current_path().string();
	
	std::fstream file(current_p + "\\am_c_integration\\extension\\manifest.json", std::fstream::out);
	file << extension_manifest;
	file.close();
	
	file.open(current_p + "\\am_c_integration\\extension\\control.js", std::fstream::out);
	file << extension_js_control;
	file.close();

	file.open(current_p + "\\am_c_integration\\extension\\playback.js", std::fstream::out);
	file << extension_js_playback;
	file.close();

	Network_Functionality nf;
	Zip_File_Handler zip;
	std::string out = std::filesystem::current_path().string() + "\\am_c_integration\\";
	std::string out_zip = out + "am_c_integration.zip";
	nf.Download_single_file("am_c_integration.zip", out + "am_c_integration.zip");

	zip.Open_Zip(out_zip.c_str());
	zip.Unzip(out.c_str());

	std::string old_out = out + "icon256.png";
	std::string new_out = out + "extension\\icon256.png";

	MoveFile(old_out.c_str(), new_out.c_str());
	DeleteFile(out_zip.c_str());

	Reopen_Elevated(std::filesystem::current_path().string() + "\\ArduinoMixer.exe", "-c_connect");
	
	System_Tray::exit_sequence();

	exit(0);
}

void Chrome_Integration::check_for_keypress() {
	if (clock.getElapsedTime().asMilliseconds() >= reset_time) {
		if (global::ci_add_hex == 0) {
			if (GetAsyncKeyState(global::ci_key_hex) < 0) {
				printf("sent!\n");
				socket.send(packet, address, port);
				clock.restart();
			}
		}
		else { 
			if (GetAsyncKeyState(global::ci_key_hex) < 0 && GetAsyncKeyState(global::ci_add_hex)) {
				printf("sent!\n");
				socket.send(packet, address, port);
				clock.restart();
			}
		}
	}
}