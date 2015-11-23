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
#define DATA_LENGTH 1016
#define FILENAME_LENGTH 80
#define WINDOW_SIZE 3
#define SEQUENCE_RANGE ((2 * WINDOW_SIZE) + 1)
#define MAX_TRIES 20

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
	int type;
	char message[RESP_LENGTH];
	bool isSuccess;
}Response;

typedef struct
{	
	char content[DATA_LENGTH];
	int length;
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
		Log(message, true);
	}

	void Log(string message, bool printConsole)
	{
		if (printConsole)
		{
			cout << message << endl;
		}

		//log to file as well
		ofstream outfile;
		outfile.open(logPath, std::ios_base::app);
		outfile << message;
		outfile.close();
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
public:
	void buildFullFilePath(char fullPath[], char fileName[])
	{
		memset(fullPath, '\0', sizeof(fullPath));
		strcpy(fullPath, dataDir);
		//strcat(fullPath, "\\");
		strcat(fullPath, fileName);
	}

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
		memset(fullPath, '\0', sizeof(fullPath));
		buildFullFilePath(fullPath, fileName);
		struct _stat stat_buf;
		return (_stat(fullPath, &stat_buf) == 0);
	}

	bool ReadFile(char fileName[FILENAME_MAX], long pos, char *buffer, int *numOfBytesRead)
	{
		char fullPath[FILENAME_MAX];
		bool endOfFile;
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
			memset(buffer, '\0', DATA_LENGTH);
			/* Read the contents of file and write into the buffer for transmission */
			fileReader.read(buffer, DATA_LENGTH);
		}
		*numOfBytesRead = fileReader.gcount();
		endOfFile = fileReader.eof();
		fileReader.close();
		return !endOfFile;
	}

	void WriteFile(char fileName[FILENAME_MAX], char buffer[DATA_LENGTH], int noOfBytes)
	{
		char fullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, fileName);
		ofstream myFile(fullPath, ios::out | ios::binary | ios::app);
		myFile.write(buffer, noOfBytes);
		myFile.close();
		return;
	}

	bool DeleteFile(char fileName[FILENAME_MAX])
	{
		char fullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, fileName);
		return (remove(fullPath) == 0);
	}

	bool RenameFile(char orginalFileName[FILENAME_MAX], char newFileName[FILENAME_MAX])
	{
		char fullPath[FILENAME_MAX], newFullPath[FILENAME_MAX];
		buildFullFilePath(fullPath, orginalFileName);
		buildFullFilePath(newFullPath, newFileName);
		return (rename(fullPath, newFullPath) == 0);
	}
};
