#ifndef CLIENT_H_
#define CLIENT_H_

#include "Epoll.h"

class Client
{
	public:
		Client();
		~Client();
//		void Write(int fd, Socket& client);
//		void Read(int fd, Socket& client) ;
//	private:
//		int room_id;
		Epoll epoll;
		Socket socket;
};

#endif
