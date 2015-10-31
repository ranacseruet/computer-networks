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
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Request
	Response res = uc->RecieveResponse();
	cout << res.message << endl;
	uc->CloseConnection();
}

void FTPClient::get()
{
	list();

	int handshake_value = handshake();
	memset(&req, '\0', sizeof(req));

	//TODO take user input and set
	string fileName;
	cout << "Type the name of file to be download: " << endl;
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	req.type = REQ_GET;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message<<endl;

	Data resData;
	ofstream myFile(fileName, ios::out | ios::binary);
	while (true)
	{
		resData = uc->RecieveData();
		myFile.write(resData.content, sizeof(resData.content));

		if (resData.isLastPacket) {
			cout << "Last Packet Received." << endl;
			break;
		}
	}
	myFile.close();

	uc->CloseConnection();
}

void FTPClient::put()
{
	int handshake_value = handshake();
	memset(&req, '\0', sizeof(req));

	string fileName;
	cout << "Type name of file to be sent: " << endl;
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	req.type = REQ_PUT;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);
}

void FTPClient::del()
{
	list();

	int handshake_value = handshake();
	memset(&req, '\0', sizeof(req));

	//taking input for sending file
	string deleteFileName;
	cout << "Type the name of file to be Deleted: " << endl;
	getline(cin, deleteFileName);
	strcpy(req.filename, deleteFileName.c_str());

	req.type = REQ_DELETE;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);


	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message << endl;
	uc->CloseConnection();
}

void FTPClient::rename()
{
	list();

	int handshake_value = handshake();
	memset(&req, '\0', sizeof(req));

	//taking input for sending file
	string fileName;
	cout << "Type file name to be renamed:    (Format: formFileName_Space_toFileName)" << endl;
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	req.type = REQ_RENAME;
	req.handshake.seq = handshake_value;

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message << endl;
	uc->CloseConnection();
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
		cin.ignore();
		list();
		break;

	case 2:
		cin.ignore();
		get();
		break;

	case 3:
		cin.ignore();
		put();
		break;

	case 4:
		cin.ignore();
		del();
		break;

	case 5:
		cin.ignore();
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

	cout << "Type name of ftp server: " << endl;
	
	while (1)
	{
		fc->showMenu();
	}
}

FTPClient::~FTPClient()
{	
}




