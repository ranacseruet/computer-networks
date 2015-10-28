#pragma once

#include "UDPClient.h"

class FTPClient 
{

private:
	UDPClient * uc;


public:
	FTPClient();

	void showMenu();
	int handshake();
	void list();
	void get();
	void put();
	void del();
	void rename();

	~FTPClient();
};