#include "FTPServer.h"
#include <iostream>

using namespace std;

int main(void)
{
	Request req;
	UDPServer *server = new UDPServer(0);
	server->RecieveRequest(&req);

	Response res;
	memset(&res, '\0', sizeof(res));
	std::string s = "Response From Server";
	strcpy(res.message, s.c_str());
	server->SendResponse(res);

	int i;
	cin >> i;
	return 0;
}