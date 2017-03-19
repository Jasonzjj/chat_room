#ifndef SERVER_H
#define SERVER_H

#include "Epoll.h"

class Server
{
public:

	Server();

//	vector <int> room_list[700];
	int room_max;
	Socket socket;
	Epoll epoll;
};

#endif
