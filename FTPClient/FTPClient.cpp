#include "FTPClient.h"

using namespace std;

FTPClient::FTPClient()
{
	uc = new UDPClient();
	uc->SetHostName("MDALIRANAEDAF");
}

int FTPClient::handshake()
{
	return 2;
}

void FTPClient::list() 
{
	int handshake_value = handshake();

	
	memset(&req, '\0', sizeof(req));

	//creating request object for listing
	std::string s = "";
	strcpy(req.filename, s.c_str());

	req.type = REQ_LIST;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->SendRequest(req);

	//Receive Request
	Response res = uc->RecieveResponse();
	cout << res.message << endl;
}

void FTPClient::get()
{
	list();

	int handshake_value = handshake();
	memset(&req, '\0', sizeof(req));

	//TODO take user input and set
	std::string s = "server_log.txt";
	strcpy(req.filename, s.c_str());

	req.type = REQ_GET;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message<<endl;

	//TODO check status and recieve data if success
}

void FTPClient::put()
{
}

void FTPClient::del()
{
}

void FTPClient::rename()
{
}

void FTPClient::showMenu() 
{
	int optionVal;
	cout << "1 : List " << endl;
	cout << "2 : GET " << endl;
	cout << "3 : PUT " << endl;
	cout << "4 : DELETE " << endl;
	cout << "5 : RENAME " << endl;
	cout << "6 : EXIT " << endl;
	cout << "Please select the operation that you want to perform : ";
	/* Check if invalid value is provided and reset if cin error flag is set */
	if (!(cin >> optionVal))
	{
		cout << endl << "Input Types does not match " << endl;
		cin.clear();
		cin.ignore(250, '\n');
	}
	/* Based on the option selected by User, set the transfer type and invoke the appropriate function */
	switch (optionVal)
	{
	case 1:
		list();
		break;

	case 2:
		get();
		break;

	case 3:
		put();
		break;

	case 4:
		del();
		break;

	case 5:
		rename();
		break;

	case 6:
		cout << "Terminating... " << endl;
		exit(1);
		break;

	default:
		cout << endl << "Please select from one of the above options " << endl;
		break;
	}
	cout << endl;
}



int main() 
{
	FTPClient * fc = new FTPClient();
	while (1)
	{
		fc->showMenu();
	}
}

FTPClient::~FTPClient()
{	
}




