#pragma comment(lib, "Ws2_32.lib")

#include <winsock.h>
#include <iostream>
#include <fstream>
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
	bool isSuccess;
}Response;

typedef struct
{	
	Handshake handshake;
	char content[RESP_LENGTH];
	bool isLastPacket;
}Data;





//Common classes, Used by both Client and Server
//**********************************************

//Logger
public class Logger
{
	string logPath;
public:
	Logger(string path)
	{
		logPath = path;
	}

	void Log(string message)
	{
		cout << message <<endl;
		
		//std::ofstream outfile;
		//outfile.open(logPath, std::ios_base::app);
		//outfile << message;
		//outfile.close();
	}
};


//File read/write helper
public class FileHelper
{
	//TODO both together suppose to be FILENAME_MAX
	char dataDir[FILENAME_MAX];
	ifstream fileReader;

	static wchar_t* charToWChar(const char* text)
	{
		size_t size = strlen(text) + 1;
		wchar_t* wa = new wchar_t[size];
		mbstowcs(wa, text, size);
		return wa;
	}

	void buildFullFilePath(char fullPath[], char fileName[])
	{
		memset(fullPath, '\0', sizeof(fullPath));
		strcpy(fullPath, dataDir);
		strcat(fullPath, "\\");
		strcat(fullPath, fileName);
	}
public:
	FileHelper(char path[FILENAME_MAX])
	{
		char currentDirectory[FILENAME_MAX];

		memset(currentDirectory, '\0', sizeof(currentDirectory));
		_getcwd(currentDirectory, sizeof(currentDirectory));

		memset(dataDir, '\0', sizeof(dataDir));
		strcat(dataDir, currentDirectory);
		strcat(dataDir, path);
	}

	string GetListOfFiles(void)
	{
		char fileName[1000];
		memset(fileName, '\0', sizeof(fileName));

		char files[100];
		memset(files, '\0', sizeof(files));

		char pathToTraverse[FILENAME_MAX];
		memset(pathToTraverse, '\0', sizeof(pathToTraverse));

		HANDLE hFind;
		WIN32_FIND_DATA data;

		strcpy(pathToTraverse, dataDir);
		strcat(pathToTraverse, "*");
		wstring stemp = charToWChar(pathToTraverse);
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

	bool DoesFileExist(char fileName[FILENAME_MAX])
	{
		char fullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, fileName);
		struct _stat stat_buf;
		return (_stat(fullPath, &stat_buf) == 0);
	}

	bool ReadFile(char fileName[FILENAME_MAX], long pos, char buffer[RESP_LENGTH])
	{
		char fullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, fileName);
		
		if (!DoesFileExist(fileName))
		{
			return false;
		}
		struct _stat stat_buf;
		_stat(fullPath, &stat_buf);
		
		fileReader.open(fullPath, ios::in | ios::binary);
		if (fileReader.is_open())
		{
			fileReader.seekg(pos);
			memset(buffer, '\0', RESP_LENGTH);
			/* Read the contents of file and write into the buffer for transmission */
			fileReader.read(buffer, RESP_LENGTH);
		}
		fileReader.close();
		return (strlen(buffer) == RESP_LENGTH);
	}

	void WriteFile(char fileName[FILENAME_MAX], char buffer[RESP_LENGTH])
	{
		char fullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, fileName);
		ofstream myFile(fullPath, ios::out | ios::binary | ios::app);
		myFile.write(buffer, strlen(buffer));
		myFile.close();
		return;
	}
};
