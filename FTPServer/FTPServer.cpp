#include "FTPServer.h"
#include <iostream>

using namespace std;

FTPServer::FTPServer(UDPServer* udp, FileHelper* helper)
{
	udpServer = udp;
	fileHelper = helper;
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
	string filesList = fileHelper->getListOfFiles();
	//cout << "Got files list: " << filesList;
	strcpy(response.message, filesList.c_str());
	udpServer->SendResponse(response);
}

int main(void)
{
	Request req;
	UDPServer *server = new UDPServer();
	FileHelper *helper = new FileHelper("\\data\\*");

	FTPServer *ftpServer = new FTPServer(server, helper);
	ftpServer->run();

	int i;
	cin >> i;
	return 0;
}