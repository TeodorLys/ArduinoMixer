#pragma once
#include <SFML/Network.hpp>
#include <SFML/System/Clock.hpp>
class Chrome_Integration {
	sf::UdpSocket socket;
	sf::Packet packet;
	sf::IpAddress address;
	sf::Clock clock;
	int reset_time = 250;
	unsigned short port;
public:
	Chrome_Integration();
	/*
	If the hotkey was pressed.
	TODO: Move this into its own class.
	*/
	void check_for_keypress();
	/*
	Writes all of the necessary files for the chrome extension.
	*/
	void Manifest_Creation();
	/*
	Creates a registry entry for communication app.
	This program will communicate with the app ^ through UDP 
	*/
	void Register_Native_Messaging_Host();
	/*
	Requests administrative privileges. i.e. It reopens it as elevated.
	*/
	void Reopen_Elevated(std::string program, std::string params, bool asElevated = true);
	/*
	The communication apps manifest creation
	*/
	void NMA_Manifest_Creation(std::string id);
	/*
	basically the install sequence for chrome addon installation.
	*/
	void Initialize_Extension_after_Install();
public:
	void set_reset_timer(int r) {
		reset_time = r;
	}
};

