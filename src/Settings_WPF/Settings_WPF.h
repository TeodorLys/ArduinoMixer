#pragma once
#include <SFML/System/Thread.hpp>
#include <atomic>
#include <mutex>
#include <Windows.h>

class Settings_WPF {
private:
	sf::Thread* t;
	std::atomic<bool> needs_updating;
	mutable std::mutex mut;
public:

	Settings_WPF() {
		t = new sf::Thread(&Settings_WPF::_render, this);
	}

	void launch() {
		t->launch();
	}

	bool get_needs_update() {
		std::lock_guard<std::mutex> l(mut);
		return needs_updating;
	}

	void confirm_update() {
		needs_updating = false;
	}

	void _render() {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		/*
		HA we have fun here! 1337 wow what fun, OMG "bro" im sooo hip eh! quirky
		*/
		CreateProcess("am_gui.exe", const_cast<char*>(std::string("w 1337_sweet_call_bro").c_str()), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

		WaitForSingleObject(pi.hProcess, INFINITE);
		
		needs_updating = true;

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		t->terminate();
	}

};
