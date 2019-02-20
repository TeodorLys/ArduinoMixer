#pragma once
#include <zip.h>
class Zip_File_Handler{
private:
	struct content_Info {
		char* content;
		int size;
	};

	zip *z;
	zip_file *f;
	struct zip_stat st;
	bool is_Open;
	int error = 0;

public:
	content_Info cn;
	char *message;
	Zip_File_Handler();
	~Zip_File_Handler();
	bool Open_Zip(const char*);
	bool Find_File(const char*);
	content_Info Get_File_Info();
	void Unzip(const char* full_path);
};

