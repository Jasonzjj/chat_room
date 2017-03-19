#include "Epoll.h"

Epoll::Epoll()
{
	epoll_fd = epoll_create(MAXEPOLLSIZE + 5);
	
	fd_num = 0;
}
bool Epoll::Add(int fd, int events_option)
{
	struct epoll_event event;
	event.events = events_option;;//EPOLLIN | EPOLLET;

	event.data.fd = fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0) 	return false;
	fd_num++;
	return true;
}

bool Epoll::Delete(int fd, int events_option)
{
	struct epoll_event event;
	event.events = events_option;;//EPOLLIN | EPOLLET;

	event.data.fd = fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, &event) < 0) 	return false;
	fd_num--;
	return true;
}

bool Epoll::Modify(int fd, int events_option)
{
	struct epoll_event event;
	event.events = events_option;;//EPOLLIN | EPOLLET;

	event.data.fd = fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event) < 0) 	return false;
	
	return true;
}
int Epoll::Wait()
{
	int number;
	number = epoll_wait(epoll_fd, events, 10, -1);
	if (number < 0) 
	{
		perror("epoll_wait");
		exit(errno);
	//	myexit("epoll_wait");
	}
	return number;
}
int Epoll::GetEpollFd()
{
	return epoll_fd;
}
int Epoll::GetFdNum()
{
	return fd_num;
}
int Epoll::GetEvents(int index)
{
	return events[index].events;
}
int Epoll::GetFd(int index)
{
	return events[index].data.fd;
}
