#include "Audio_Controller.h"
#include "crash_logger.h"
#include <Windows.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include "Network_Functionality.h"

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

std::vector<Audio_Volume_Variables> Audio_Controller::sliders;

std::string Audio_Controller::Get_Session_Display_Name(IAudioSessionControl **pSessionControl) {
	HRESULT hr = S_OK;
	LPWSTR try_Str = 0;
	std::string display;  //What to display.
	(*pSessionControl)->GetDisplayName(&try_Str);
	std::wstring temp = try_Str;
	if (temp.find(L"AudioSrv") == std::wstring::npos) {
		IAudioSessionControl2 *PID;  //For the the PID(process id)
		(*pSessionControl)->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&PID);

		DWORD max = MAX_PATH;
		DWORD t2 = 0;
		char buffer[MAX_PATH];

		hr = PID->GetProcessId(&t2);

		HANDLE hprocess = NULL;

		if (t2 != 0)
			hprocess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, t2);

		if (hprocess != NULL) {
			QueryFullProcessImageName(hprocess, 0, buffer, &max);  //Will get the actual process path(including the NAME).
			display = buffer;
			try {
				display.erase(0, display.find_last_of("\\") + 1);  //Removes the path, excluding the progam name.
				display.erase(display.size() - 4, display.size());  // --||--
			}
			catch (std::exception e) {
				printf("%s\n", e.what());
				crash_logger ci;
				ci.log_message(e.what(), __FUNCTION__);
				display = "UNPARSE";
			}
		}
		else {
			return "NULL";
		}

		std::transform(display.begin(), display.end(), display.begin(), ::toupper);
		std::string buff = ""; 
		for (char c : display) {
			if (c != (char)32)
				buff += c;
		}

		display = buff;

		if (display.size() > 6)
			display.erase(6, display.size());
		else if (display.size() < 6) {
			int save_Size = display.size();
			for (int a = 0; a < 6 - save_Size; a++) {
				if (a % 2 == 0) {
					display = display + " ";
				}
				else if (a % 2 == 1) {
					display = " " + display;
				}
			}
		}
		SAFE_RELEASE(PID);
		if (hprocess != NULL)
			CloseHandle(hprocess);
	}
	else {
		display = "SYSTEM";
	}
	return display;
}

bool Audio_Controller::Check_for_Restrictions(std::string s) {
	for (std::string ex : global::to_Exclude) {
		if (s.find(ex) != std::string::npos) {
			return false;
		}
	}
	return true;
}

int Audio_Controller::Check_for_Reservations(std::string s) {
	for (reserved r : global::reserved_List) {
		if (r.name.find(s) != std::string::npos)
			return r.index;
		else if(s.find(r.name) != std::string::npos)
			return r.index;
	}
	return -1;
}

std::string Audio_Controller::Check_For_New_Name(std::string s) {
	for (rename_session rs : global::renamed) {
		if (s.find(rs.default_name) != std::string::npos)
			return rs.new_name;
	}
	return s;
}

Audio_Volume_Variables Audio_Controller::Enumarate_Audio_Session(int index) {
	Audio_Volume_Variables buff;
	HRESULT hr = 0;
	hr = pSessionEnum->GetSession(index, &pSessionControl);
	hr = pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)& buff.control);
	buff.display_Name = Get_Session_Display_Name(&pSessionControl);
	buff.control->GetMasterVolume(&buff.last_Value);
	buff.control->GetMute((BOOL*)& buff.mute);
	if (buff.display_Name == "NULL") {
		buff.display_Name = Get_Session_Display_Name(&pSessionControl);
		if (buff.display_Name == "NULL") {
			std::string text = "ERROR CODE" + std::to_string(hr);
			crash_logger ci;
			ci.log_message(text + ", Getting session display name", __FUNCTION__);
			buff.display_Name = "UNKNOWN";
		}
	}
	return buff;
}

Audio_Controller::Audio_Controller(AudioSession* v1, AudioSession* v2, AudioSession* v3, AudioSession* v4) {
	HRESULT hr = S_OK;

	//Save the potentiometers value pointers.
	save_Vars[0] = v1;
	save_Vars[1] = v2;
	save_Vars[2] = v3;
	save_Vars[3] = v4;

	hr = CreateSessionManager(&pSession);

	if (hr != S_OK) {
		std::string text = "ERROR CODE: " + std::to_string(hr);
		crash_logger ci;
		ci.log_message(text + ", Creating session manager", __FUNCTION__);
		MessageBox(NULL, text.c_str(), "FATAL ERROR HAS OCCURED", MB_OK);
		exit(-1);
	}

	hr = pSession->GetSessionEnumerator(&pSessionEnum);

	if (hr != S_OK) {
		std::string text = "ERROR CODE: " + std::to_string(hr);
		crash_logger ci;
		ci.log_message(text + ", Getting session enumerator", __FUNCTION__);
		MessageBox(NULL, text.c_str(), "FATAL ERROR HAS OCCURED", MB_OK);
		exit(-1);
	}

	hr = pSessionEnum->GetCount(&old_Count);
	if (hr != S_OK) {
		std::string text = "ERROR CODE: " + std::to_string(hr);
		crash_logger ci;
		ci.log_message(text + ", Getting session count", __FUNCTION__);
		MessageBox(NULL, text.c_str(), "FATAL ERROR HAS OCCURED", MB_OK);
		exit(-1);
	}

	Audio_Volume_Variables buff;
	std::vector<Audio_Volume_Variables> temp;

	/*
	Get all active audio sessions, Process name and current volume.
	*/
	for (int a = 0; a < old_Count; a++) {
		buff = Enumarate_Audio_Session(a);
		format_sessions::get().assign_session_spot(a, buff.display_Name);
		if (Check_for_Restrictions(buff.display_Name)) {
			temp.push_back(buff);
		}
		SAFE_RELEASE(buff.control);
		SAFE_RELEASE(pSessionControl);
	}

	std::vector<Audio_Volume_Variables> format_temp(36);

	/*
	This loop adds the reserved controls, 
	and removes them from the "TEMP" vector.
	*/
	for (reserved r : global::reserved_List) {
		if (Check_for_Restrictions(r.name)) {
			if(r._option_index == 1)
				format_temp[r.index].display_Name = r.name;
			else
				format_temp[r.index].display_Name = "RESERV";  // Pre-set the reserved spots, 
																										 //if any of the controls are active it will be overwritten.
			for (int a = 0; a < (int)temp.size(); a++) {
				if (temp[a].display_Name.find(r.name) != std::string::npos) {
					format_temp[r.index] = temp[a];
					try {
						temp.erase(temp.begin() + a);
					}
					catch (std::exception &e) {
						crash_logger ci;
						ci.log_message(e.what(), __FUNCTION__);
					}
				}
			}
		}
	}

	int otemp = 0;

	/*This loop adds the remaining controls(with no reservation registered)*/
	for (int a = 0; a < 36; a++) {
		if (otemp < (int)temp.size()) {
			if (format_temp[a].display_Name == "") {
				format_temp[a] = temp[otemp]; /*<-*/
				otemp++;
			}
		}
		else {
			if (format_temp[a].display_Name == "") {
				format_temp[a].display_Name = "UNUSED";
			}
		}
		/* 
		Sets the pot pointer after the reservation checking, 
		because this can be overwritten by -> 
		*/
		format_temp[a].v = &save_Vars[a % 4]->value;
	}

	/*
	Pushes all of the display names to a global vector, for other classes and such.
	Giving the formated structure, of display name and values to the main structure
	*/
	for (int a = 0; a < 36; a++) {
		global::programs.push_back(Check_For_New_Name(format_temp[a].display_Name));
		sliders.push_back(format_temp[a]);
		*sliders[sliders.size() - 1].v = sliders[sliders.size() - 1].last_Value;
	}


	/*
	Dont know why I did this... but changing it at this time will be 
	a major restructure...
	Save the index of the object, and the page it is on.
	*/
	for (int a = 1; a <= 9; a++) {
		for (int b = 0; b < 4; b++) {
			sliders[((a - 1) * 4) + b].page_index = a;
			sliders[((a - 1) * 4) + b].index = ((a - 1) * 4) + b;
		}
	}
	/*
	Setting all page options in the main array.
	*/
	for (reserved r : global::reserved_List) {
		sliders[r.index].option_index = r._option_index;
		printf("Changing: %i to %i\n", r.index, sliders[r.index].option_index);
	}

}/*---</CONSTRUCTOR>---*/

void Audio_Controller::Rerender_Audio_Displays() {
	int program_Open = -1;

	for (reserved r : global::reserved_List) {
		// Don't do any checks or edits if it's restricted/excluded.
		if (Check_for_Restrictions(r.name)) {
			/*
			When there is a program occuping the newly reserved spot we will
			move it to a first unused spot.
			If it is not occupied, it will be reserved or replaced with the active
			audio session.
			*/
			if (sliders[r.index].display_Name != "UNUSED" && sliders[r.index].display_Name != "RESERV" && sliders[r.index].display_Name != r.name && r._option_index < 1) {
				bool moved = false;
				for (int a = r.index; a < (int)sliders.size(); a++) {
					if (sliders[a].display_Name == "UNUSED" && !moved) {
						sliders[a].display_Name = sliders[r.index].display_Name;
						global::programs[a] = Check_For_New_Name(sliders[r.index].display_Name);
						sliders[a].control = sliders[r.index].control;
						moved = true;
					}
					//Saves the spot of which the reserved program is.
					if (sliders[a].display_Name == r.name) {
						program_Open = a;
					}
				}
				/*
				If the reserved program is open, it will be changed to the new spot.
				And all of the assets will be handed over.
				*/
				if (program_Open != -1) {
					sliders[r.index].display_Name = sliders[program_Open].display_Name;
					global::programs[r.index] = Check_For_New_Name(sliders[program_Open].display_Name);
					sliders[r.index].control = sliders[program_Open].control;
					//Releases the old spot for the reserved program.
					sliders[program_Open].display_Name = "UNUSED";
					SAFE_RELEASE(sliders[program_Open].control);
				}
				/*
				If the progam was not open, just change the spot to RESERV.
				*/
				else {
					sliders[r.index].display_Name = "RESERV";
					global::programs[r.index] = "RESERV";
				}
			}
			/*
			Changes a currently UNUSED spot to the newly reseved spot,
			And hands over all gathered assets.
			*/
			else if (sliders[r.index].display_Name == "UNUSED") {
				sliders[r.index].display_Name = "RESERV";
				global::programs[r.index] = "RESERV";
				for (int a = 0; a < (int)sliders.size(); a++) {
					if (sliders[a].display_Name == r.name) {
						sliders[r.index].display_Name = sliders[a].display_Name;
						global::programs[r.index] = sliders[a].display_Name;
						sliders[r.index].control = sliders[a].control;
						break;
					}
				}
			}
			/*
			*NEW!*
			if a page has been reserved for extra features, i.e. not audio control.

			*PENDING*
			This is where the chaiscript will be allocated and reserved for a spot.
			*/
			else if (r._option_index == 1) {
				sliders[r.index].option_index = r._option_index;
				printf("this is a option index 1, %s\n", sliders[r.index].display_Name.c_str());
			}
		}
		/*
		If the newly added reservation is excluded, it will only be excluded, 
		i.e. exclution takes priority!
		*/
		else {
			//printf("restricted...\n");
			sliders[r.index].display_Name == "UNUSED";
			sliders[r.index].last_Value = 0;
			SAFE_RELEASE(sliders[r.index].control);
			global::programs[r.index] = "UNUSED";
		}
	}

	for (int a = 0; a < (int)sliders.size(); a++) {
		/*
		Checks if and of the current available sessions 
		are newly restricted.
		And releases all assets for the session.
		*/
		if (!Check_for_Restrictions(sliders[a].display_Name)) {
			sliders[a].display_Name = "UNUSED";
			sliders[a].last_Value = 0;
			global::programs[a] = "UNUSED";
			SAFE_RELEASE(sliders[a].control);
			continue;
		}/*--*/

		/*
		Checks if a spot has been released from its reservation.
		And releases all assets for the session
		*/
		if (sliders[a].display_Name == "RESERV") {
			bool found = false;
			for (reserved r : global::reserved_List) {
				if (a == r.index) {
					found = true;
					break;
				}
			}
			if (!found) {
				printf("not found...\n");
				sliders[a].display_Name = "UNUSED";
				sliders[a].last_Value = 0;
				global::programs[a] = "UNUSED";
			}
		} /*--*/
	}
	/*
	Checks if there are any new names for sessions.
	*/
	for (int a = 0; a < (int)sliders.size(); a++) {
		global::programs[a] = Check_For_New_Name(sliders[a].display_Name);
	}
	Push_Back_Audio_Controller();
}/*---</Rerender_Audio_Displays>---*/

//New EXCLUTION based pop back
void Audio_Controller::Pop_Back_Audio_Controller() {
	format_sessions::get().push_sessions_to_buffer();
	session _removed = format_sessions::get().get_recently_closed_program();

	printf("Removed: %s\n", _removed.name.c_str());

	for (int a = 0; a < 36; a++) {
		if (format_sessions::get().compare_session(_removed.name, a)) {
			format_sessions::get().deallocate_session(a);
			break;
		}
	}

	format_sessions::get().reconstruct_sessions();

	printf("\nPOP_BACK\n");
	for (int a = 0; a < (int)sliders.size(); a++) {
		if (sliders[a].display_Name != "UNUSED") {
			SAFE_RELEASE(sliders[a].control);
		}
	}

	CreateSessionManager(&pSession);
	pSession->GetSessionEnumerator(&pSessionEnum);

	std::string removed = "";

	int b = 0;
	pSessionEnum->GetCount(&b);
	Audio_Volume_Variables buff;
	std::vector<Audio_Volume_Variables> temp;
	
	for (int a = 0; a < b; a++) {
		buff = Enumarate_Audio_Session(a);
		buff.v = &save_Vars[a % 4]->value;
		if (Check_for_Restrictions(buff.display_Name))
			temp.push_back(buff);
		SAFE_RELEASE(buff.control);
		SAFE_RELEASE(pSessionControl);
	}
	
	std::string compare = "";

	for (int a = 0; a < (int)temp.size(); a++)
		compare += temp[a].display_Name;

	//Check which is gone...
	for (int a = 0; a < (int)sliders.size(); a++) {
		if (compare.find(sliders[a].display_Name) == std::string::npos && sliders[a].display_Name != "UNUSED" && sliders[a].display_Name != "RESERV") {
			removed = sliders[a].display_Name;
			break;
		}
	}

	for (int a = 0; a < 36; a++) {
		if (sliders[a].display_Name != removed) {
			for (int b = 0; b < (int)temp.size(); b++) {
				if (sliders[a].display_Name == temp[b].display_Name) {
					sliders[a].control = temp[b].control;
					sliders[a].last_Value = temp[b].last_Value;
					sliders[a].mute = temp[b].mute;
					break;
				}
			}
		}
		else if(sliders[a].display_Name == removed){
			int r = Check_for_Reservations(removed);
			if (r == -1) {
				global::programs[a] = "UNUSED";
				sliders[a].display_Name = "UNUSED";
				sliders[a].last_Value = 0;
			}
			//When the spot is reserved
			else {
				global::programs[r] = "RESERV";
				sliders[r].display_Name = "RESERV";
				sliders[r].last_Value = 0;
			}
		}
	}
		SAFE_RELEASE(pSession);
		CoUninitialize();
}/*---</Pop_Back_Audio_Controller>---*/

void Audio_Controller::Push_Back_Audio_Controller() {
	printf("\nPUSH_BACK\n");

	format_sessions::get().push_sessions_to_buffer();

	session _added = format_sessions::get().get_recently_opened_program();

	if (format_sessions::get().is_reserverd(_added.name)) {
		for (int a = 0; a < format_sessions::get().get_reserv_size(); a++) {
			if (format_sessions::get().get_reserv_session(a).name == _added.name) {
				format_sessions::get().get(format_sessions::get().get_reserv_session(a).index).name = format_sessions::get().get_reserv_session(a).name;
			}
		}
	}
	else {
		format_sessions::get().assign_next_unused_spot(_added.name);
	}

	format_sessions::get().reconstruct_sessions();

	/*
	Releases all of the controls, will rewrite them later on.
	Otherwise there will be an access violation,
	when I try to find the newly added program, I need to re:enumerate the controls
	which makes the "old" ones invalid
	*/
	for (int a = 0; a < (int)sliders.size(); a++) {
		if (sliders[a].display_Name != "UNUSED" && sliders[a].display_Name != "RESERV")
			SAFE_RELEASE(sliders[a].control);
	}

	CreateSessionManager(&pSession);
	pSession->GetSessionEnumerator(&pSessionEnum);

	std::string compare = "";

	/*
	For easy element checking, I add all display names into a string, 
	and compare to the added, to check which was added.
	*/
	for (int a = 0; a < (int)sliders.size(); a++)
				if (sliders[a].display_Name != "UNUSED" && sliders[a].display_Name != "RESERV") {
					compare += sliders[a].display_Name;
	}

	int new_Count = 0;  // new session count
	Audio_Volume_Variables buff;
	std::vector<Audio_Volume_Variables> temp;  //Save all current audio sessions
	pSessionEnum->GetCount(&new_Count);

	/*
	Reenumarate the audio sessions, and get the display names
	*/
	for (int a = 0; a < new_Count; a++) {
		buff = Enumarate_Audio_Session(a);
		if (Check_for_Restrictions(buff.display_Name)) // "OLD!!" buff.display_Name.find("SYSTEM") == std::string::npos
			temp.push_back(buff);
		SAFE_RELEASE(buff.control);
		SAFE_RELEASE(pSessionControl);
	}

	std::string added = "";

	//Check which is added...
	for (int a = 0; a < (int)temp.size(); a++) {
		if (compare.find(temp[a].display_Name) == std::string::npos && temp[a].display_Name != "UNUSED") {
			added = temp[a].display_Name;
		}
	}
	//

	bool has_added = false;
	int r = Check_for_Reservations(added);
	
	/*
	If the newly started program has a reserved spot
	*/
	if (r != -1) {
		for (Audio_Volume_Variables v : temp) {
			if (v.display_Name == added) {
				sliders[r].display_Name = v.display_Name;
				sliders[r].control = v.control;
				sliders[r].last_Value = v.last_Value;
				sliders[r].mute = v.mute;
				global::programs[r] = Check_For_New_Name(v.display_Name);
			}
		}
		has_added = true;
	}

	for (int a = 0; a < new_Count; a++) {
		/*when the spot is reserved, it will be skipped and add 1 to the counter.*/
		if (sliders[a].display_Name == "RESERV")
			new_Count++;
		else if (sliders[a].option_index != 0) {
			new_Count++;
			continue;
		}

		/*
		Adds the newly started program to the controller, and sets all assets,
		for the spot.
		*/
		if (sliders[a].display_Name == "UNUSED" && !has_added) {
			for (int b = 0; b < (int)temp.size(); b++) {
				if (temp[b].display_Name == added) {
					sliders[a].display_Name = temp[b].display_Name;
					sliders[a].control = temp[b].control;
					sliders[a].last_Value = temp[b].last_Value;
					sliders[a].mute = temp[b].mute;
					global::programs[a] = Check_For_New_Name(temp[b].display_Name);
					has_added = true;
					break;
				}
			}
			continue;
		}

		/*Reassign the enumurated volume controls*/
		for (int b = 0; b < (int)temp.size(); b++) {
			if (sliders[a].display_Name == temp[b].display_Name) {
				sliders[a].control = temp[b].control;
				sliders[a].last_Value = temp[b].last_Value;
				sliders[a].mute = temp[b].mute;
				break;
			}
		}
	}
	SAFE_RELEASE(pSession);
	CoUninitialize();
} /*---</Push_Back_Audio_Controller>---*/

Audio_Controller::~Audio_Controller() {
	SAFE_RELEASE(pSession);
	SAFE_RELEASE(pSessionEnum);
	SAFE_RELEASE(pSessionControl);
}

HRESULT Audio_Controller::CreateSessionManager(IAudioSessionManager2** ppSessionManager) {
	HRESULT hr = S_OK;

	IMMDevice* pDevice = NULL;
	IMMDeviceEnumerator* pEnumerator = NULL;
	IAudioSessionManager2* pSessionManager = NULL;
	hr = CoInitialize(pEnumerator);
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);

	if (hr != S_OK) {
		printf("Could not CoCreateInstance\n");
		SAFE_RELEASE(pDevice);
		SAFE_RELEASE(pEnumerator);
		SAFE_RELEASE(pSessionManager);
		return 1;
	}

	hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);

	hr = pDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_ALL, NULL, (void**)&pSessionManager);

	*(ppSessionManager) = pSessionManager;
	(*ppSessionManager)->AddRef();

	SAFE_RELEASE(pDevice);
	SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pSessionManager);
	return hr;
}

void Audio_Controller::Free_All() {
	CoUninitialize();
	SAFE_RELEASE(pSessionEnum);
	SAFE_RELEASE(pSessionControl); 
	SAFE_RELEASE(pSession);
}

bool Audio_Controller::Set_Volume() {
	for (int c = 0; c < (int)sliders.size(); c++) {
		if (sliders[c].page_index == global::page_Number && sliders[c].control != NULL && save_Vars[0]->has_changed) {
			if (sliders[c].option_index == 0) {
				if (sliders[c].v == nullptr) {
					crash_logger cl;
					cl.log_message_with_last_error(__FUNCTION__);
					exit(0);
				}
				if (*sliders[c].v >= 0) {
					sliders[c].control->SetMasterVolume(*sliders[c].v, NULL);
				}
				else if (*sliders[c].v >= -0.91 && *sliders[c].v <= -0.88) {
					sliders[(4 * (global::page_Number - 1)) + 0].control->SetMute((BOOL)!sliders[(4 * (global::page_Number - 1)) + 0].mute, NULL);
					sliders[(4 * (global::page_Number - 1)) + 0].mute = !sliders[(4 * (global::page_Number - 1)) + 0].mute;
				}
				else if (*sliders[c].v == -0.80) {
					sliders[(4 * (global::page_Number - 1)) + 1].control->SetMute((BOOL)!sliders[(4 * (global::page_Number - 1)) + 1].mute, NULL);
					sliders[(4 * (global::page_Number - 1)) + 1].mute = !sliders[(4 * (global::page_Number - 1)) + 1].mute;
				}
				else if (*sliders[c].v == -0.70) {
					sliders[(4 * (global::page_Number - 1)) + 2].control->SetMute((BOOL)!sliders[(4 * (global::page_Number - 1)) + 2].mute, NULL);
					sliders[(4 * (global::page_Number - 1)) + 2].mute = !sliders[(4 * (global::page_Number - 1)) + 2].mute;
				}
				else if (*sliders[c].v == -0.60) {
					sliders[(4 * (global::page_Number - 1)) + 3].control->SetMute((BOOL)!sliders[(4 * (global::page_Number - 1)) + 3].mute, NULL);
					sliders[(4 * (global::page_Number - 1)) + 3].mute = !sliders[(4 * (global::page_Number - 1)) + 3].mute;
				}
			}
		}
		/*
		ONLY FOR TESTING!!!
		*/
		else if(sliders[c].page_index == global::page_Number && sliders[c].option_index > 0 && save_Vars[0]->has_changed){
			if (*sliders[c].v >= 0.f) {
				int r = static_cast<int>(*sliders[c].v * 100.f);
				int g = static_cast<int>(*sliders[c + 1].v * 100.f);
				int b = static_cast<int>(*sliders[c + 2].v * 100.f);
				Network_Functionality::send_request(r, g, b);
				break;
			}
		}
	}
	int buff = Get_Count();

	if (old_Count != buff) {
		Free_All();
		if (old_Count > buff)
			Pop_Back_Audio_Controller();
		else
			Push_Back_Audio_Controller();
		old_Count = buff;
		return false;
	}
	else {
		return true;
	}
}

float Audio_Controller::Get_Volume(int index) const {
	float buff;
	sliders[index].control->GetMasterVolume(&buff);
	return buff;
}

int Audio_Controller::Get_Count() {
	int b = old_Count;
	if (std::chrono::duration_cast<std::chrono::milliseconds>(tend - tstart).count() >= 500) {
		IAudioSessionEnumerator *pEnum = 0;
		IAudioSessionManager2 *pSez = 0;
		CreateSessionManager(&pSez);
		pSez->GetSessionEnumerator(&pEnum);

		pEnum->GetCount(&b);
		
		SAFE_RELEASE(pEnum);
		SAFE_RELEASE(pSez);
		CoUninitialize();
		tstart = std::chrono::high_resolution_clock::now();
	}
	tend = std::chrono::high_resolution_clock::now();
	return b;
}