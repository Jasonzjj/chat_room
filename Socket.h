#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <string>
#include <stdlib.h>
#include <cstdio>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include <vector>
#include <iostream>
#include <assert.h>

using namespace std;

const int MAXCONNECTION = 15;
const int MAXRECEIVE = 500;



class Socket
{
	public:
		Socket();
		virtual ~Socket();

		bool Create();
		bool Bind(const char* ip, const int port);
		
		bool Listen() const;
		bool Accept(Socket& client_socket) const ;
		
		bool Connect(const string host, const int port);

	//	bool Send(Socket& socket, const string message) const;
	//	int Receive(Socket& socket, const string message) const;

//		void SetNonBlock(const bool flag);
		bool IsValid() const;

//	private:
		char buf[102];
		int sock_fd;
		struct sockaddr_in address;
		int room_id;
		int cmd;
};

#endif
