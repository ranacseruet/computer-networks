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
		request = udpServer->RecieveRequest();
		
		cout << "Got request type: " << request.type;
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
			break;
		case REQ_RENAME:
			rename(request);
			break;
		case REQ_DELETE:
			del(request);
			break;
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
	cout << "Here sending response" << endl;
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
	int numOfBytesRead = 0;
	while (1)
	{
		memset(dataStream, '\0', RESP_LENGTH);
		memset(data.content, '\0', RESP_LENGTH);
		bool lastPacket = !fileHelper->ReadFile(request.filename, pos, dataStream, &numOfBytesRead);
		memcpy(data.content, dataStream, RESP_LENGTH);
		data.isLastPacket = lastPacket;
		udpServer->SendData(data);
		//cout << "File read:" << strlen(dataStream) << " bytes" << endl;
		if (lastPacket)
		{
			break;
		}
		pos += numOfBytesRead;
	}
	logger->Log("\nFile sending complete. Total Bytes: ");
	cout << pos << endl;
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
		//cout << "File data recieved:" << strlen(data.content) << " bytes" << endl;

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
	Response response;
	memset(&response, '\0', sizeof(response));

	if (fileHelper->DoesFileExist(request.filename))
	{
		if (fileHelper->DeleteFile(request.filename))
		{
			response.isSuccess = true;
			strcpy(response.message, "File successfully deleted");
		}
		else
		{
			strcpy(response.message, "File exist, but couldn't be deleted");
			response.isSuccess = false;
		}
	}
	else
	{
		strcpy(response.message, "File does not exist");
		response.isSuccess = false;
	}

	udpServer->SendResponse(response);
}

void FTPServer::rename(Request request)
{
	Response response;
	memset(&response, '\0', sizeof(response));

	if (fileHelper->DoesFileExist(request.filename))
	{
		if (fileHelper->RenameFile(request.filename, request.renamedFileName))
		{
			response.isSuccess = true;
			strcpy(response.message, "File successfully renamed");
		}
		else
		{
			strcpy(response.message, "File exist, but couldn't be renamed");
			response.isSuccess = false;
		}
	}
	else
	{
		strcpy(response.message, "File does not exist");
		response.isSuccess = false;
	}

	udpServer->SendResponse(response);
}

int main(void)
{
	Request req;
	Logger *logger = new Logger("data\\server_log.txt");
	UDPServer *server = new UDPServer(logger);
	FileHelper *helper = new FileHelper("\\server_data\\");
	FTPServer *ftpServer = new FTPServer(server, helper, logger);
	ftpServer->run();

	//Data data = server->RecieveData();
	//cout << data.content<<endl;

	int i;
	cin >> i;
	return 0;
}