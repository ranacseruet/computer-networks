#include "FTPClient.h"

using namespace std;

FTPClient::FTPClient()
{
	uc = new UDPClient();
	fh = new FileHelper("\\client_data\\");
	char logFilePath[FILENAME_LENGTH+100] = { '\0' };
	_getcwd(logFilePath, sizeof(logFilePath));
	fh->buildFullFilePath(logFilePath, "client_log.txt");
	logger = new Logger(logFilePath);
}

int FTPClient::handshake()
{
	int random = (rand() % 100) + 1;

	memset(&handsahke, '\0', sizeof(handsahke));
	
	handsahke.seq = random;

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "Handshake Started with SEQ# %d\n", random);
	logger->Log(logMessage);

	//sending list request
	uc->CreateConnection();
	uc->SendHandshakeRequest(handsahke);

	//Receive Request
	Handshake handsahke = uc->RecieveHandshakeResponse();

	
	memset(logMessage, '\0', sizeof(logMessage));
	sprintf(logMessage, "From server ACK# %d\n", handsahke.seq);
	logger->Log(logMessage);
	uc->CloseConnection();
	return handsahke.seq+1;
}

void FTPClient::list() 
{
	int handshake_value = handshake();
	
	memset(&req, '\0', sizeof(req));
	//creating request object for listing
	std::string s = "";
	strcpy(req.filename, s.c_str());

	req.type = REQ_LIST;
	req.handshake.ack = handshake_value;

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "List request sent with ACK# %d\n", handshake_value);
	logger->Log(logMessage);

	uc->CloseConnection();

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Request
	Response res = uc->RecieveResponse();
	logger->Log("List Response recieved from the server");
	cout << res.message << endl;
	uc->CloseConnection();
}

void FTPClient::get()
{
	list();

	int handshake_value = handshake();

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "GET request with ACK# %d\n", handshake_value);
	logger->Log(logMessage);

	memset(&req, '\0', sizeof(req));

	//TODO take user input and set
	string fileName;
	cout << "Type the name of file to be download: " << endl;
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	req.type = REQ_GET;
	req.handshake.ack = handshake_value;

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message << endl;

	if (!res.isSuccess) 
	{
		uc->CloseConnection();
		return;
	}
	while (true)
	{
		Data resData;
		resData = uc->RecieveData();
		cout << resData.length << endl;
		fh->WriteFile(req.filename, resData.content, resData.length);
		if (resData.isLastPacket) {
			cout << "Last Packet Received." << endl;
			break;
		}
	}

	logger->Log("File Received.");
	uc->CloseConnection();
}

void FTPClient::put()
{
	FileHelper * fh = new FileHelper("\\client_data\\");
	cout << fh->GetListOfFiles() << endl;
	

	string fileName;
	cout << "Type name of file to be sent: " << endl;
	memset(&req, '\0', sizeof(req));
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	int handshake_value = handshake();
	req.type = REQ_PUT;
	req.handshake.ack = handshake_value;

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "PUT request with ACK# %d\n", handshake_value);
	logger->Log(logMessage);

	
	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message << endl;

	if (!res.isSuccess) 
	{
		uc->CloseConnection();
		return;
	}


	char dataStream[DATA_LENGTH];
	memset(dataStream, '\0', DATA_LENGTH);
	Data data;
	long pos = 0;
	while (1)
	{
		memset(dataStream, '\0', DATA_LENGTH);
		memset(data.content, '\0', DATA_LENGTH);
		int numOfBytesRead = 0;
		bool lastPacket = !fh->ReadFile(req.filename, pos, dataStream, &numOfBytesRead);
		memcpy(data.content, dataStream, numOfBytesRead);
		data.isLastPacket = lastPacket;
		data.length = numOfBytesRead;
		uc->SendData(data);
		//cout << "File read:" << strlen(dataStream) << " bytes" << endl;
		if (lastPacket)
		{
			cout << "This was last packet"<<endl;
			break;
		}
		pos += numOfBytesRead;
	}


	logger->Log("File sent.");
	uc->CloseConnection();
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
	req.handshake.ack = handshake_value;

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "DELETE request with ACK# %d\n", handshake_value);
	logger->Log(logMessage);

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
	string fileName, newFileName;
	cout << "Type file name to be renamed:" << endl;
	getline(cin, fileName);
	strcpy(req.filename, fileName.c_str());

	cout << "Type new name for the file :" << endl;
	getline(cin, newFileName);
	strcpy(req.renamedFileName, newFileName.c_str());

	req.type = REQ_RENAME;
	req.handshake.ack = handshake_value;

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "Rename request with ACK# %d\n", handshake_value);
	logger->Log(logMessage);

	//sending list request
	uc->CreateConnection();
	uc->SendRequest(req);

	//Receive Response
	Response res = uc->RecieveResponse();
	cout << res.message << endl;
	uc->CloseConnection();
}

void FTPClient::run()
{
	char  fileName[FILENAME_MAX];
	cout << "Type name of ftp server: " << endl;
	cin >> fileName;
	uc->SetHostName(fileName);
	cin.clear();

	while (1)
	{
		showMenu();
	}
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
	fc->run();
	
	//strcpy(data.content, "Hello World With Content Greater than 20 bytes jalksdjlaksjdklajsdkljaskldjaskldjaskldjaskldjaskldjaslkTheEnd");
	/*UDPClient *udpClient = new UDPClient();
	udpClient->SetHostName("MDALIRANAEDAF");
	udpClient->CreateConnection();


	FileHelper *fh = new FileHelper("\\client_data\\");
	char dataStream[DATA_LENGTH];
	memset(dataStream, '\0', DATA_LENGTH);
	long pos = 0;
	while (1)
	{
		Data data;
		memset(dataStream, '\0', DATA_LENGTH);
		memset(data.content, '\0', DATA_LENGTH);
		int numOfBytesRead = 0;
		bool lastPacket = !fh->ReadFile("test3.jpg", pos, dataStream, &numOfBytesRead);
		cout << "Bytes read: " << numOfBytesRead << endl;
		memcpy(data.content, dataStream, numOfBytesRead);
		data.isLastPacket = lastPacket;
		udpClient->SendData(data);
		//cout << "File read:" << strlen(dataStream) << " bytes" << endl;
		if (lastPacket)
		{
			cout << "This was last packet" << endl;
			break;
		}
		pos += numOfBytesRead;
	}

	//udpClient->SendData(data);
	udpClient->CloseConnection();*/
	int i;
	cin >> i;
}

FTPClient::~FTPClient()
{	
}




