#pragma once

#include "UDPClient.h"

class FTPClient 
{

private:
	UDPClient * uc;
	Request req;
	string serverIpAdd;
	Handshake handsahke;
	FileHelper * fh;
	Logger *logger;
	string fileList;

public:
	FTPClient();

	void run();
	void showMenu();
	int handshake();
	void list();
	void get();
	void put();
	void del();
	void rename();

	~FTPClient();
};