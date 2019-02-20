#include "Audio_Control.h"
#include <Windows.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

std::string Audio_Control::Get_Session_Display_Name(IAudioSessionControl **pSessionControl) {
	LPWSTR try_Str = 0;
	std::string display;  //What to display.
	(*pSessionControl)->GetDisplayName(&try_Str);
	std::wstring temp = try_Str;
	if (temp.find(L"AudioSrv") == std::wstring::npos) {
		IAudioSessionControl2 *PID;  //To the the PID(process id)
		(*pSessionControl)->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&PID);

		DWORD max = MAX_PATH;
		DWORD t2 = 0;
		char buffer[MAX_PATH];

		PID->GetProcessId(&t2);

		HANDLE hprocess = NULL;

		if (t2 != 0)
			hprocess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, t2);

		if (hprocess != NULL) {
			QueryFullProcessImageName(hprocess, 0, buffer, &max);  //Will get the actual process path(including the NAME).
			display = buffer;
			display.erase(0, display.find_last_of("\\") + 1);  //Removes the path, excluding the progam name.
			display.erase(display.size() - 4, display.size());  // --||--
		}
		std::transform(display.begin(), display.end(), display.begin(), ::toupper);

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

bool Audio_Control::Check_for_Restrictions(std::string s) {
	for (std::string ex : global::to_Exclude) {
		if (s.find(ex) != std::string::npos) {
			return false;
		}
	}
	return true;
}

int Audio_Control::Check_for_Reservations(std::string s) {
	for (reserved r : global::reserved_List) {
		if (r.name.find(s) != std::string::npos) {
			return r.index;
		}
	}
	return -1;
}

std::string Audio_Control::Check_For_New_Name(std::string s) {
	for (rename_session rs : global::renamed) {
		if (s.find(rs.default_name) != std::string::npos)
			return rs.new_name;
	}
	return s;
}

Audio_Control::Audio_Control(float* v1, float* v2, float* v3, float* v4) {
	tstart = std::chrono::high_resolution_clock::now();  //For the checking when a new audio session is added.

	//Save the potentiometers value pointers.
	save_Vars[0] = v1;
	save_Vars[1] = v2;
	save_Vars[2] = v3;
	save_Vars[3] = v4;

	CreateSessionManager(&pSession);
	pSession->GetSessionEnumerator(&pSessionEnum);

	pSessionEnum->GetCount(&old_Count);
	int temp_Index = 1;  // pre-defined page number in the vector
	int offset = 0;
	Audio_Volume_Variables buff;
	std::vector<Audio_Volume_Variables> temp;

	for (int a = 0; a < old_Count; a++) {
		pSessionEnum->GetSession(a, &pSessionControl);
		pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&buff.control);
		buff.display_Name = Get_Session_Display_Name(&pSessionControl);

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
			format_temp[r.index].display_Name = "RESERV";  // Pre-set the reserved spots, 
																										 //if any of the controls are active it will be overwritten.
			for (int a = 0; a < (int)temp.size(); a++) {
				if (temp[a].display_Name == r.name) {
					format_temp[r.index] = temp[a];
					temp.erase(temp.begin() + a);
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
		format_temp[a].v = save_Vars[a % 4];
	}

	for (int a = 0; a < 36; a++) {
		global::programs.push_back(Check_For_New_Name(format_temp[a].display_Name));
		sliders.push_back(format_temp[a]);
	}

	for (int a = 1; a <= 9; a++) {
		for (int b = 0; b < 4; b++) {
			sliders[((a - 1) * 4) + b].index = a;
		}
	}
}

void Audio_Control::Rerender_Audio_Displays() {
	int program_Open = -1;

	for (reserved r : global::reserved_List) {
		printf("name: %s\n", r.name.c_str());
		// Don't do any checks or edits if it's restricted/excluded.
		if (Check_for_Restrictions(r.name)) {
			/*
			When there is a program occuping the newly reserved spot we will
			move it to a first unused spot.
			If it is not occupied, it will be reserved or replaced with the active
			audio session.
			*/
			if (sliders[r.index].display_Name != "UNUSED" && sliders[r.index].display_Name != "RESERV" && sliders[r.index].display_Name != r.name) {
				bool moved = false;
				for (int a = r.index; a < sliders.size(); a++) {
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
				if (program_Open != -1) {
					sliders[r.index].display_Name = sliders[program_Open].display_Name;
					global::programs[r.index] = Check_For_New_Name(sliders[program_Open].display_Name);
					sliders[r.index].control = sliders[program_Open].control;
					//Releases the old spot for the reserved program.
					sliders[program_Open].display_Name = "UNUSED";
					SAFE_RELEASE(sliders[program_Open].control);
				}
				else {
					printf("Reserv\n");
					sliders[r.index].display_Name = "RESERV";
					global::programs[r.index] = "RESERV";
				}
			}
			else if (sliders[r.index].display_Name == "UNUSED") {
				sliders[r.index].display_Name = "RESERV";
				global::programs[r.index] = "RESERV";
				for (int a = 0; a < sliders.size(); a++) {
					if (sliders[a].display_Name == r.name) {
						sliders[r.index].display_Name = sliders[a].display_Name;
						global::programs[r.index] = sliders[a].display_Name;
						sliders[r.index].control = sliders[a].control;
						break;
					}
				}
			}
		}
		/*
		If the newly added reservation is excluded, it will only be excluded, 
		i.e. exclution takes priority!
		*/
		else {
			printf("restricted...\n");
			sliders[r.index].display_Name == "UNUSED";
			SAFE_RELEASE(sliders[r.index].control);
			global::programs[r.index] = "UNUSED";
		}
	}

	/*
	Checks if there is any new exclutions added.
	If so it will release them and, change it to UNUSED
	*/
	for (int a = 0; a < sliders.size(); a++) {
		if (!Check_for_Restrictions(sliders[a].display_Name)) {
			printf("Newly restricted\n");
			sliders[a].display_Name = "UNUSED";
			global::programs[a] = "UNUSED";
			SAFE_RELEASE(sliders[a].control);
			continue;
		}
		if (sliders[a].display_Name == "RESERV") {
			bool found = false;
			for (reserved r : global::reserved_List) {
				printf("found at: %i, %s\n", r.index, r.name.c_str());
				if (a == r.index) {
					found = true;
					break;
				}
			}
			if (!found) {
				printf("not found...\n");
				sliders[a].display_Name = "UNUSED";
				global::programs[a] = "UNUSED";
			}
		}
	}
	for (int a = 0; a < sliders.size(); a++) {
		global::programs[a] = Check_For_New_Name(sliders[a].display_Name);
	}
	Push_Back_Audio_Control();
}

//New EXCLUTION based pop back
void Audio_Control::Pop_Back_Audio_Control() {
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
		pSessionEnum->GetSession(a, &pSessionControl);
		pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&buff.control);
		buff.display_Name = Get_Session_Display_Name(&pSessionControl);

		buff.v = save_Vars[a % 4];
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
					break;
				}
			}
		}
		else if(sliders[a].display_Name == removed){
			int r = Check_for_Reservations(removed);
			if (r == -1) {
				global::programs[a] = "UNUSED";
				sliders[a].display_Name = "UNUSED";
			}
			//When the spot is reserved
			else {
				global::programs[r] = "RESERV";
				sliders[r].display_Name = "RESERV";
			}
		}
	}
		SAFE_RELEASE(pSession);
		CoUninitialize();
}

void Audio_Control::Push_Back_Audio_Control() {
	printf("\nPUSH_BACK\n");

	/*Releases all of the controls, will rewrite then later on.
		Otherwise there will be an access violation,
		when I try to find the newly added program, I need to re:enumerate the controls
		which makes the "old" ones invalid*/
	for (int a = 0; a < (int)sliders.size(); a++) {
		if (sliders[a].display_Name != "UNUSED" && sliders[a].display_Name != "RESERV")
			SAFE_RELEASE(sliders[a].control);
	}

	CreateSessionManager(&pSession);
	pSession->GetSessionEnumerator(&pSessionEnum);

	std::string compare = "";

	for (int a = 0; a < (int)sliders.size(); a++)
				if (sliders[a].display_Name != "UNUSED" && sliders[a].display_Name != "RESERV") {
					compare += sliders[a].display_Name;
	}

	int new_Count = 0;  // new session count
	Audio_Volume_Variables buff;
	std::vector<Audio_Volume_Variables> temp;  //Save all current audio sessions
	pSessionEnum->GetCount(&new_Count);

	for (int a = 0; a < new_Count; a++) {
		pSessionEnum->GetSession(a, &pSessionControl);
		pSessionControl->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&buff.control);
		buff.display_Name = Get_Session_Display_Name(&pSessionControl);
		if (Check_for_Restrictions(buff.display_Name)) // "OLD!!" buff.display_Name.find("SYSTEM") == std::string::npos
			temp.push_back(buff);
		SAFE_RELEASE(buff.control);
		SAFE_RELEASE(pSessionControl);
	}

	std::string added = "";

	//Check which is addded...
	for (int a = 0; a < (int)temp.size(); a++) {
		if (compare.find(temp[a].display_Name) == std::string::npos && temp[a].display_Name != "UNUSED") {
			added = temp[a].display_Name;
		}
	}
	//

	bool has_added = false;
	int r = Check_for_Reservations(added);
	
	if (r != -1) {
		printf("r: %i, %s\n", r, added.c_str());
		for (Audio_Volume_Variables v : temp) {
			if (v.display_Name == added) {
				sliders[r].display_Name = v.display_Name;
				sliders[r].control = v.control;
				global::programs[r] = Check_For_New_Name(v.display_Name);
			}
		}
		has_added = true;
	}

	for (int a = 0; a < new_Count; a++) {
		/*when the spot is reserved, it will be skipped and add 1 to the counter.*/
		if (sliders[a].display_Name == "RESERV")
			new_Count++;

		if (sliders[a].display_Name == "UNUSED" && !has_added) {
			for (int b = 0; b < (int)temp.size(); b++) {
				if (temp[b].display_Name == added) {
					sliders[a].display_Name = temp[b].display_Name;
					sliders[a].control = temp[b].control;
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
				break;
			}
		}
	}
	SAFE_RELEASE(pSession);
	CoUninitialize();
}

Audio_Control::~Audio_Control() {
	SAFE_RELEASE(pSession);
	SAFE_RELEASE(pSessionEnum);
	SAFE_RELEASE(pSessionControl);
}

HRESULT Audio_Control::CreateSessionManager(IAudioSessionManager2** ppSessionManager) {
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

void Audio_Control::Free_All() {
	CoUninitialize();
	SAFE_RELEASE(pSessionEnum);
	SAFE_RELEASE(pSessionControl);
	SAFE_RELEASE(pSession);
}

bool Audio_Control::Set_Volume() {
	for (int c = 0; c < (int)sliders.size(); c++) {
		if (sliders[c].index == global::page_Number && sliders[c].control != NULL) {
			if (sliders[c].v == nullptr) {
				/*Should probably not use exceptions but... wanted to try something else:)*/
				throw ("SLIDER VARIABLE POINTER WAS NULL %i", c);
			}
			sliders[c].control->SetMasterVolume(*sliders[c].v - 0.005f, NULL);
		}
	}
	int buff = Get_Count();

	if (old_Count != buff) {
		Free_All();
		if (old_Count > buff)
			Pop_Back_Audio_Control();
		else
			Push_Back_Audio_Control();
		old_Count = buff;
		return false;
	}
	else {
		return true;
	}
}

float Audio_Control::Get_Volume(int index) const {
	float buff;
	sliders[index].control->GetMasterVolume(&buff);
	return buff;
}

int Audio_Control::Get_Count() {
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