#include "Global_Variables.h"
#include "crash_logger.h"
#include "update_handler.h"
#include <myio.h>
#include "BalloonTip.h"
#include <filesystem>
#include "Zip_File_Handler.h"
#include "device_IO.h"
#include "console.h"

bool update_handler::check_for_updates(bool quiet_mode) {
	std::string out = net.download_single_stream("VERSION.txt");

	std::string temp;

	for (char c : out) {
		if (c != '\n')
			temp += c;
		else
			break;
	}
	std::string server_Version = temp;
	int iversion = myio::stoiEX(global::_version) + 1;
	int iserversion = myio::stoiEX(server_Version) + 1;
	if (net.last_update() == "")
		net.last_update() = temp;
	else if (iserversion > iversion) {
		printf("Update FOUND!\n");
		BalloonTip::_Set_Identifier(1);
		BalloonTip::Call_BallonTip("UPDATE AVAILABLE!", "Found an update, click to Install");
		net.last_update() = temp;
		/*
		Maybe I shouldnt download it as soon as it is avaliable, but it is pretty small.
		TODO: a option in the settings menu?
		*/
		download_updates();
		return true;
	}
	else {
		printf("Everything is up to date!\n");
		/*
		If quiet_mode is true, it means that the program desided to check
		for updates(for ex. time based). if it's false, it means that the
		user clicked the "Check for updates" button.
		i.e. no notification will be sent if quiet mode is active!
		*/
		if (!quiet_mode) {
			BalloonTip::_Set_Identifier(2);
			BalloonTip::Call_BallonTip("Everything is up to date!", "No update found");
		}
		net.last_update() = temp;
		return false;
	}
	return false;
}

bool update_handler::download_updates(){
	std::string filename = net.last_update();
	if (filename.find(".zip") != std::string::npos)
		filename += ".zip";

	update_path = std::filesystem::temp_directory_path().string() + "\\" + filename;

	return net.Download_single_file(filename, update_path);
}

bool update_handler::install_updates() {
	std::string temp_dir = std::filesystem::temp_directory_path().string();

	Zip_File_Handler zip;
	
	zip.Open_Zip(update_path.c_str());
	zip.Unzip(temp_dir.c_str());  // Unpacks the files into the temp folder in appdata
	if (std::filesystem::exists(temp_dir + "\\UPDATE_DEVICE")) {
		device_IO dio;
		/*
		finds the current firmware file by hardware version. sooo this will use the query ex. "firmware_4"
		and it will return a list of all firmware for hardware version 4.
		*/
		std::vector<std::string> list = net.list_files_in_dropbox("firmware_" + dio.get_device_info().hardware_version);
		std::string firmware;
		if (list.size() > 1) {
			std::string last;
			for (std::string s : list) {
				try {
					/*
					Parses the software version ex firmware_4.1.hex
					                                     Here ^
				  for all found firmware versions
					*/
					s.erase(0, s.find(".") + 1);
					s.erase(s.find("."), s.length());
				}
				catch (std::exception& e) {
					crash_logger cl;
					cl.log_message(e.what() + std::string(" Could not parse firmware version..."), __FUNCTION__);
					goto skip_firmware;
				}
				/*
				Finds the latest version...
				*/
				if (last != "") {
					int c_i = stoi(s);  // current in the list
					int l_i = stoi(last); // last parsed version
					if (c_i > l_i)
						firmware = s;
				}
				else
					last = s;
			}
		}
		else {
			/*
			If there only exists one on the server, like right now... 2020-04-19(firmware_4.1.hex)
			*/
			firmware = list[0];
		}
		std::string public_version = firmware;
		try {
			public_version.erase(0, public_version.find(".") + 1);
			public_version.erase(public_version.find("."), public_version.length());
		}
		catch (std::exception& e) {
			crash_logger cl;
			cl.log_message(e.what() + std::string(" Could not parse the public firmware version"), __FUNCTION__);
			goto skip_firmware;
		}

		int i_p_version = stoi(public_version);
		int i_l_version = stoi(dio.get_device_info().software_version);

		/*
		Here we compare the current installed firmware in the device to the server version
		*/
		if (i_p_version > i_l_version) {
			net.Download_single_file(firmware, temp_dir + "\\" + firmware);
			std::string f_path = temp_dir + "\\" + firmware;
			int r_message = MessageBox(NULL, "A new device firmware was found\nInstall?", "FIRMWARE UPDATE", MB_OKCANCEL);
			if (r_message == IDOK)
				install_device_firmware(f_path);
		}
	}

	skip_firmware:

	try {
		std::filesystem::remove(update_path);
	}
	catch (std::filesystem::filesystem_error& e) {
		crash_logger cl;
		cl.log_message(e.what() + std::string("Could not remove the zip update file!"), __FUNCTION__);
	}

	std::fstream info_File(temp_dir + "\\info_temp_file.txt", std::fstream::binary | std::fstream::out);
	info_File << std::filesystem::current_path().string() << std::endl;
	info_File << temp_dir << std::endl; // Why not...?
	info_File << global::documents_Settings_Path << std::endl;
	info_File.close();

	std::string updater = temp_dir + "\\Updater.exe";
	ShellExecute(NULL, "runas", updater.c_str(), 0, 0, SW_SHOWNORMAL);
	HANDLE _mutex = OpenMutex(MUTEX_ALL_ACCESS, 0, "Local\\$arduino$mixer$");
	ReleaseMutex(_mutex);
	exit(0);
}

bool update_handler::install_device_firmware(std::string firmware) {
	//SYNTAX: tycmd upload --board @COM8 firmware.hex
	console _console;
	_console.allocate_console();

	if (firmware.find("firmware") == std::string::npos)
		return false;

	if (firmware.find(".hex") == std::string::npos)
		firmware += ".hex";

	if (!std::filesystem::exists("tycmd")) {
		crash_logger cl;
		cl.log_message("could not find tycmd, not installed?", __FUNCTION__);
		printf("could not find tycmd, updater failed?\n");
		return false;
	}

	std::string command = "tycmd upload --board @" + device_IO::com_port() + " " + firmware;

	/*
	TODO: use something different to "system".
	NOT GOOD!!!
	*/
	system(command.c_str());

	return true;

}

bool update_handler::finish_and_clean_install() {
	global::waiting_Update = false;
	std::string temp = std::filesystem::temp_directory_path().string();
	try {
		std::filesystem::remove(temp + "\\Updater.exe");
		std::filesystem::remove(temp + "\\info_temp_file.txt");
		std::filesystem::remove(global::documents_Settings_Path + "\\RECENTLY_UPDATED.txt");
	}
	catch (const std::filesystem::filesystem_error& f) {
		printf("Could not delete updater files: %s\n", f.what());
		crash_logger cl;
		cl.log_message(f.what() + std::string("could not remove the install files... its fine though"), __FUNCTION__);
		return false;
	}
	return true;
}