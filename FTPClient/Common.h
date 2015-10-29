#pragma comment(lib, "Ws2_32.lib")

#include <winsock.h>
#include <iostream>
#include <string>
#include <windows.h>
#include <sys/stat.h>
#include <direct.h>

#define SERVER_PORT 5001

#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 140
#define FILENAME_LENGTH 80
#define BUFFER_LENGTH 256
//BUFFER_LENGTH is largest one

using namespace std;

typedef enum
{
	REQ_LIST = 1,
	REQ_GET = 2,
	REQ_PUT = 3,
	REQ_DELETE = 4,
	HANDSHAKE = 5,
	REQ_RENAME = 6,
} Type;

typedef struct
{
	int seq;
	int ack;
	int nak;
} Handshake; //For Put Operatio

typedef struct
{
	Handshake handshake;
	int type;
	char filename[FILENAME_LENGTH];
	char renamedFileName[FILENAME_LENGTH];
}Request;

typedef struct
{
	Handshake handshake;
	int type;
	char message[RESP_LENGTH];
}Response;

typedef struct
{	
	Handshake handshake;
	char content[RESP_LENGTH];
}Data;



public class FileHelper
{
	string dirPath;
	
	wstring s2ws(const std::string& s)
	{
		int len;
		int slength = (int)s.length() + 1;
		len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
		wchar_t* buf = new wchar_t[len];
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
		std::wstring r(buf);
		delete[] buf;
		return r;
	}
public:
	FileHelper(string path)
	{
		dirPath = path;
	}

	string getListOfFiles(void)
	{
		char cCurrentPath[FILENAME_MAX], fileName[1000];
		char files[100];
		memset(files, '\0', sizeof(files));
		_getcwd(cCurrentPath, sizeof(cCurrentPath));

		HANDLE hFind;
		WIN32_FIND_DATA data;
		std::wstring stemp = s2ws(strcat(cCurrentPath, dirPath.c_str()));
		LPCWSTR rootPath = stemp.c_str();
		hFind = FindFirstFile(rootPath, &data);
		if (hFind != INVALID_HANDLE_VALUE) {
			int i = 0;
			do {
				//wprintf_s(data.cFileName);
				wcstombs(fileName, data.cFileName, sizeof(data.cFileName));
				if (strcmp(fileName, ".") != 0 && strcmp(fileName, "..") != 0)
				{
					memcpy((files + strlen(files)), fileName, strlen(fileName));
					strcat(files, "\n");
					i++;
				}
			} while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
		else {
			cout << "invalid handle value: " << hFind;
		}
		string str(files);
		return str;
	}
};
