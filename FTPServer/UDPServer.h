#include "../FTPClient/UDP.h"

public class UDPServer: public UDP
{
private:
	Logger *logger;
public:
	UDPServer(Logger*);
	~UDPServer(void);

	bool SendResponse(Response data);
	bool RecieveRequest(Request*);
};