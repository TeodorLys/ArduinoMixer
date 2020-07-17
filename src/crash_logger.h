#pragma once
#include <string>
#include <vector>
#include <fstream>
#include "Global_Variables.h"

/*
Production diary... kind of

Hello, I am writing this at 8 pm 2020-04-18 as a general documentation
of the current situation about this project and the world i guess...

Why am I writing this in here? well this is probably the least looked at class/file
its just a crash logger.

anyways, as i wrote above, the situation of the world. Right now, we/I am in the current
pandemic of COVID-19. I have luckily not contracted the virus(yet), so I am obviously 
quarantining myself, not much different to normal, but still a active effort :)

So... this project have been under development since late 2018, alittle over 1 year.
and it has grown alot! I have grown alot. The program are currently in alpha version 2.5.0(unreleased. 2.2.6 are public),
and firmware alpha version 1(didnt start version control until very recently...).

The actual hardware are at version 4, and a OTG(on the go) v.1. I am thinking about releasing this as a product.
But Im not sure how, and if there are any intresse in it. But you dont know if you dont try, Right?

Ok. I think I am done there, whom ever might be reading this(Probably just me, Teodor). This was just
me, literally, letting my mind leak out all over the keyboard. I have to clean this up now...

HA funny jokes here!

ok, bye. good luck
*/

class crash_logger {
private:
	std::fstream file;
public:
	crash_logger();
	/*
	Logs windows last error message to a crashlog within the documents data map.
	*/
	void log_message_with_last_error(std::string extra = "", const char* func = "");
	/*
	Logs a message to a crashlog within the documents data map.
	*/
	void log_message(std::string s, const char* func);
};

