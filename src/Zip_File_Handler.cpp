#include "Zip_File_Handler.h"
#include <fstream>
#include <zip.h>

Zip_File_Handler::Zip_File_Handler() {}

Zip_File_Handler::~Zip_File_Handler() {
	delete[] cn.content;
}

bool Zip_File_Handler::Open_Zip(const char* path) {
	z = zip_open(path, 0, &error);
	if (error > 0) {
		printf("Zip file could not be opened, Error code: %i\n", error);
		return false;
	}
	else {
		printf("Zip file %s opened\n", path);
		is_Open = true;
		return true;
	}
}

void Zip_File_Handler::Unzip(const char* full_path) {
	char buff[100];
	for (int a = 0; a < zip_get_num_entries(z, 0); a++) {
		if (zip_stat_index(z, a, 0, &st) == 0) {
			f = zip_fopen_index(z, a, 0);
			std::string file_name = full_path;
			file_name += st.name;
			std::fstream file(file_name, std::fstream::binary | std::fstream::out);

			int sum = 0;
			int len = 0;
			while (sum != st.size) {
				len = (int)zip_fread(f, buff, 100);
				file.write(buff, len);
				sum++;
			}
			file.close();
			zip_fclose(f);
		}
	}
	zip_close(z);
}

bool Zip_File_Handler::Find_File(const char* file_Name) {
	if (is_Open) {
		zip_stat_init(&st);
		if (zip_stat(z, file_Name, 0, &st) < 0) {
			printf("File cound not be found...\n");
			return false;
		}
		else {
			cn.content = new char[static_cast<int>(st.size)];
			cn.size = static_cast<int>(st.size);
			f = zip_fopen(z, file_Name, 0);
			zip_fread(f, cn.content, st.size);
			zip_fclose(f);
			printf("%s was found\n", file_Name);
			return true;
		}
	}
	else {
		printf("Zip file has not been opened\n");
		return false;
	}
}

Zip_File_Handler::content_Info Zip_File_Handler::Get_File_Info() {
	return cn;
}