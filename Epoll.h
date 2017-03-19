#ifndef EPOLL_H_
#define EPOLL_H_

#include "Socket.h"
#include <sys/epoll.h>
#include <sys/resource.h>

const int MAXEPOLLSIZE = MAXCONNECTION + 5;

class Epoll
{
	public:
		Epoll();
		bool Add(int fd, int events_option);
		int Wait();
		bool Delete(int fd, int events_option);
		bool Modify(int fd, int events_option);
		int GetEpollFd();
		int GetFdNum();
		int GetEvents(int index);
		int GetFd(int index);
//	private:
		int fd_num;
		int epoll_fd;
		
		struct epoll_event events[MAXEPOLLSIZE];
};
#endif
