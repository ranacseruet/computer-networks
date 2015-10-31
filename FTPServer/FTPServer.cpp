#include "FTPServer.h"
#include <iostream>

using namespace std;

FTPServer::FTPServer(UDPServer* udp, FileHelper* helper, Logger* log)
{
	udpServer = udp;
	fileHelper = helper;
	logger = log;
}

void FTPServer::run()
{
	while (1)
	{
		Request request;
		bool success = udpServer->RecieveRequest(&request);
		if (!success)
		{
			//error occured
			break;
		}

		switch (request.type)
		{
		case REQ_LIST:
			list(request);
			break;
		case REQ_GET:
			get(request);
			break;
		case REQ_PUT:
			put(request);
		case REQ_RENAME:
			rename(request);
		case REQ_DELETE:
			del(request);
		default:
			break;
		}
	}
}

void FTPServer::list(Request request)
{
	Response response;
	memset(&response, '\0', sizeof(response));
	string filesList = fileHelper->GetListOfFiles();
	if (strlen(filesList.c_str()) <= 0)
	{
		strcpy(response.message, "No files exist in server");
		response.isSuccess = false;
	}
	else 
	{
		strcpy(response.message, filesList.c_str());
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);
	logger->Log("Responsed to LIST operation");
}

void FTPServer::get(Request request)
{
	//1. Send success/failure response
	Response response;
	memset(&response, '\0', sizeof(response));
	if (!fileHelper->DoesFileExist(request.filename))
	{
		strcpy(response.message, "File doesn't exist");
		response.isSuccess = false;
	}
	else
	{
		strcpy(response.message, "File found. Server is going to send data");
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);
	
	if (!response.isSuccess)
	{
		//no data to send
		return;
	}

	//send data
	char dataStream[RESP_LENGTH];
	memset(dataStream, '\0', sizeof(dataStream));
	Data data;
	long pos = 0;
	while (1)
	{
		memset(dataStream, '\0', RESP_LENGTH);
		bool lastPacket = !fileHelper->ReadFile(request.filename, pos, dataStream);
		strcpy(data.content, dataStream);
		data.isLastPacket = lastPacket;
		udpServer->SendData(data);
		cout << "File read:" << strlen(dataStream) << " bytes" << endl;
		if (lastPacket)
		{
			break;
		}
		pos += strlen(dataStream);
	}
	logger->Log("\nFile sending complete");
}

void FTPServer::put(Request request)
{
	//1. Send success/failure response
	Response response;
	memset(&response, '\0', sizeof(response));
	if (fileHelper->DoesFileExist(request.filename))
	{
		strcpy(response.message, "File already exist. Delete that first");
		response.isSuccess = false;
	}
	else
	{
		strcpy(response.message, "Server is ready to recieve data");
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);

	if (!response.isSuccess)
	{
		//no data to send
		return;
	}

	//recieve data
	Data data;
	long pos = 0;
	while (true)
	{
		data = udpServer->RecieveData();
		fileHelper->WriteFile(request.filename, data.content);
		cout << "File data recieved:" << strlen(data.content) << " bytes" << endl;

		if (data.isLastPacket)
		{
			cout << "This was last packet" << endl;
			break;
		}
	}
	logger->Log("\nFile data recieving complete");
}

void FTPServer::del(Request request)
{
	//TODO
}

void FTPServer::rename(Request request)
{
	//TODO
}

int main(void)
{
	Request req;
	Logger *logger = new Logger("data\\server_log.txt");
	UDPServer *server = new UDPServer(logger);
	FileHelper *helper = new FileHelper("\\server_data\\");
	FTPServer *ftpServer = new FTPServer(server, helper, logger);
	ftpServer->run();

	int i;
	cin >> i;
	return 0;
}