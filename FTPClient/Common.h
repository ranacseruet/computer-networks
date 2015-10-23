#pragma comment(lib, "Ws2_32.lib")

#include <winsock.h>

#define HOSTNAME_LENGTH 20
#define RESP_LENGTH 40
#define FILENAME_LENGTH 20
#define SERVER_PORT 5001
#define BUFFER_LENGTH 256
#define MAXPENDING 10
#define MSGHDRSIZE 8
#define ACK_LENGHT 3

typedef enum
{
	REQ_LIST = 1,
	REQ_GET = 2,
	REQ_PUT = 3,
	REQ_DELETE = 4,
	HANDSHAKE = 5,
	REQ_RENAME = 6,
} Type;

typedef struct
{
	int seq;
	int ack;
	int nak;
} Handshake; //For Put Operatio

typedef struct
{
	Handshake handshake;
	int type;
	char filename[FILENAME_LENGTH];
	char renamedFileName[FILENAME_LENGTH];
}Request;

typedef struct
{
	Handshake handshake;
	int type;
	char message[FILENAME_LENGTH];
}Response;

typedef struct
{	
	Handshake handshake;
	char conetent[BUFFER_LENGTH];
}Data;


