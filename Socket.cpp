#include "Socket.h"

Socket::Socket()
{
	sock_fd = room_id = -1;
	memset(buf, 0, sizeof(buf));
}
Socket:: ~Socket()
{
	if (IsValid())
	{
		close(sock_fd);
	}
}

//server
bool Socket::Create()
{
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (!IsValid()) return false;
	return true;
}
bool Socket::Bind(const char* ip, const int port)
{
	if (!IsValid()) return false;
	address.sin_family = AF_INET;
	inet_pton(AF_INET, ip, &address.sin_addr);
//	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port = htons(port);
	if (bind(sock_fd, (struct sockaddr*)&address, sizeof(address)) == -1) return false;
	return true;
}
bool Socket::Listen()const 
{
	if (!IsValid()) return false;
	int listen_fd = listen(sock_fd, MAXCONNECTION);
	if (listen_fd == -1) return false;
	return true;
}

bool Socket::Accept(Socket& client_socket)const
{
	socklen_t client_addr_len = sizeof(client_socket.address);
	//....

	client_socket.sock_fd = accept(sock_fd, (struct sockaddr*)&client_socket.address, &client_addr_len);
	
	if (client_socket.sock_fd == -1) return false;
	return true;
}

//client
bool Socket::Connect(const string host, const int port)
{
	if (!IsValid()) return false;
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = inet_addr(host.c_str());

	if (connect(sock_fd, (struct sockaddr*)&address, sizeof(address)) == -1) return false;
	return true;
}
/*
void Socket:: SetNonBlock(const bool flag)
{
	if (IsValid())
	{
		int opts = fcntl(sock_fd, F_GETFL);
		if (opts < 0 ) return ;
		if (flag) opts |= O_NONBLOCK;
		else opts &= (~O_NONBLOCK);

		fcntl(sock_fd, F_SETFL, opts);
	}
}
*/
bool Socket::IsValid() const 
{
	return sock_fd != -1;
}

