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
		udpServer->RecieveRequest(&request);
		switch (request.type)
		{
		case REQ_LIST:
		case REQ_GET:
			list(request);
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
	strcpy(response.message, filesList.c_str());
	udpServer->SendResponse(response);
	logger->Log("Responsed to LIST operation with message: " + filesList);
}

int main(void)
{
	Request req;
	Logger *logger = new Logger("data\\server_log.txt");
	UDPServer *server = new UDPServer(logger);
	FileHelper *helper = new FileHelper("\\data\\*");

	FTPServer *ftpServer = new FTPServer(server, helper, logger);
	ftpServer->run();

	int i;
	cin >> i;
	return 0;
}