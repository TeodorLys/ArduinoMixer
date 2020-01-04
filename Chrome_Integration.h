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
	void check_for_keypress();
	void Manifest_Creation();
	void Register_Native_Messaging_Host();
	void Reopen_Elevated(std::string program, std::string params, bool asElevated = true);
	void NMA_Manifest_Creation(std::string id);
public:
	void set_reset_timer(int r) {
		reset_time = r;
	}
};

