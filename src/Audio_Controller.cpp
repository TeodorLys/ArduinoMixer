#include "Audio_Controller.h"
#include "crash_logger.h"
#include <Windows.h>
#include <audiopolicy.h>
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <stdio.h>
#include <algorithm>
#include <iostream>
#include "network_handler.h"


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

bool Audio_Controller::is_renamed(std::string s) {
	for (std::string rs : global::rename_list) {
		s.erase(std::remove_if(s.begin(), s.end(), ::isspace), s.end());
		if (rs.find(s) != std::string::npos)
			return true;
	}
	return false;
}

std::string Audio_Controller::check_for_default_name(std::string s) {
	for (rename_session rs : global::renamed) {
		if (s.find(rs.new_name) != std::string::npos)
			return rs.default_name;
	}
	return s;
}

Audio_Controller::Audio_Controller(AudioSession* v1, AudioSession* v2, AudioSession* v3, AudioSession* v4) {
	HRESULT hr = S_OK;

	//Save the potentiometers value pointers.
	save_Vars[0] = v1;
	save_Vars[1] = v2;
	save_Vars[2] = v3;
	save_Vars[3] = v4;

	format_sessions::get().set_volume_pointers(save_Vars);

	format_sessions::get().construct_sessions();

	old_Count = format_sessions::get().recent_buffer_size();
}/*---</CONSTRUCTOR>---*/

void Audio_Controller::Rerender_Audio_Displays() {
	printf("rerender\n");

	format_sessions::get().push_sessions_to_buffer();


	for (int a = 0; a < 36; a++) {
		if (!Check_for_Restrictions(format_sessions::get().get(a).name) && format_sessions::get().get(a).name != "UNUSED" && format_sessions::get().get(a).name != "RESERV") {
			format_sessions::get().free_reserv_spot(a);
			format_sessions::get().deallocate_session(a);
		}
		if(is_renamed(format_sessions::get().get(a).name))
			format_sessions::get().get(a).name = Check_For_New_Name(format_sessions::get().get(a).name);
		else 
			format_sessions::get().get(a).name = check_for_default_name(format_sessions::get().get(a).name);
	}

	for (int a = 0; a < format_sessions::get().get_reserv_size(); a++) {
		if (Check_for_Restrictions(format_sessions::get().get_reserv_session(a).name)) {
			session_reservation reservation = format_sessions::get().get_reserv_session(a);
			std::string name_of_spot = check_for_default_name(format_sessions::get().get(reservation.index).name);
			if(name_of_spot == "RESERV")
				name_of_spot = check_for_default_name(format_sessions::get().get(reservation.index).reserved_name);
			if (reservation.name != name_of_spot) {
				format_sessions::get().deallocate_session(format_sessions::get().get_index(reservation.name));
				format_sessions::get().assign_next_unused_spot(format_sessions::get().get(reservation.index).name);
				if(format_sessions::get().active_in_buffer(reservation.name))
					format_sessions::get().assign_session_spot(reservation.index, reservation.name);
				else 
					format_sessions::get().assign_session_spot(reservation.index, "RESERV");

				printf("%s\n", format_sessions::get().get(reservation.index).name.c_str());

			}
		}
	}

	format_sessions::get().reenumerate_sessions();
	Push_Back_Audio_Controller();
}/*---</Rerender_Audio_Displays>---*/

//New EXCLUTION based pop back
void Audio_Controller::Pop_Back_Audio_Controller() {
	format_sessions::get().push_sessions_to_buffer();
	session _removed = format_sessions::get().get_recently_closed_program();

	if (_removed.name == "excluded") {
		return;
	}

	printf("Removed: %s\n", _removed.name.c_str());

	for (int a = 0; a < 36; a++) {
		if (format_sessions::get().compare_session(_removed.name, a)) {
			format_sessions::get().deallocate_session(a);
			break;
		}
	}

	format_sessions::get().reenumerate_sessions();

	printf("\nPOP_BACK\n");
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
		if (Check_for_Restrictions(_added.name))
			format_sessions::get().assign_next_unused_spot(_added.name);
	}

	format_sessions::get().reenumerate_sessions();
} /*---</Push_Back_Audio_Controller>---*/

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

bool Audio_Controller::Set_Volume() {
	if(save_Vars[0]->has_changed)
		format_sessions::get().set_active_volume_parameters();

	int buff = Get_Count();

	if (old_Count != buff) {
		//Free_All();
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