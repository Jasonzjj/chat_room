#include "server.h"
const int N = 70123;

Server::Server()
{
	socket = *(new Socket());
	epoll = *(new Epoll());
	room_max = 0;
	
}
vector <int> room_list[777];

void SetNonBlock(int fd, const bool flag)
{
	int opts = fcntl(fd, F_GETFL);
	if (opts < 0) return ;
	if (flag) opts |= O_NONBLOCK;
	else opts &= (~O_NONBLOCK);

	fcntl(fd, F_SETFL, opts);
}
int StringToDig(string s)
{
	int num = 0;
	for (int i = 0; i < s.length() && s[i] != '\n'; ++i)
	{
		if (s[i] < '0' || s[i] > '9') return -1;
		num = num * 10 + s[i] - '0';	
	
	}
	return num;
}
string DigToString(int num)
{
	string s = "";
	if (num == 0) return "0";
	while (num)
	{
		int x = num % 10;
		num /= 10;
		s += (x + '0');
	}
	string ts = "";
	//printf("--%d\n", s.length());
	for (int i = (int)s.length() - 1; i >= 0; --i) ts += s[i];
	return ts;
}
struct Node
{
	int magic_num;
	int cmd_id;
	int crc32;
	int length;

	string msg;
}node;

char buf[103333];
char buf2[102333];
string s, msg;

int Port[N];
int Room[N];

int Hash(string s)
{
	int hash = 0;
	int mod = 13337;
	for (int i = 0; i < s.length(); ++i)
	{
		hash += s[i];
	}
	hash %= mod;
	hash = (hash + mod) % mod;
	return hash;
}
bool DecodeMsg(const string &data, int &cmd_id, string &msg)
{


	
	memset(buf, 0, sizeof(buf));
	
	//	strcpy(buf, data.c_str());
//	printf("--%d\n", data.length());	
	memcpy(buf, data.c_str(), data.length());
//	puts("ook");	
	//for (int i = 0; i < data.length(); ++i) buf[i] = data[i];
	
	int magic_num = *(int *)&buf[0];
	
	cmd_id = *(int *)&buf[4];
	
	int crc32 = *(int *)&buf[8];
	
	int len = *(int *)&buf[12];
	
	//msg = buf + 16;
//	cout << msg << endl;
//	printf("len = %d\n", len);	
//	if (len) puts("cao");
	if (len) msg = string(buf + 16, len);
		
//	puts("fick");
	if (Hash(msg) != crc32) return false;
	
	if (len != msg.length()) return false;
	
	return true;


}


bool EncodeMsg(const int cmd_id, const string &data, string &msg)

{

	memset(buf, 0, sizeof(buf));


	
	int magic_num = 1;
	
	int crc32 = Hash(data);
	
	int length = data.length();
	
	*(int *)&buf[0] = magic_num;
	
	*(int *)&buf[4] = cmd_id;
	
	*(int *)&buf[8] = crc32;
	
	*(int *)&buf[12] = length;


	
	memcpy(buf + 16, data.c_str(), length);


	
	msg = string(buf, 16 + length);
	
	return true;
}


void handle_accept(Server& server)

{

	//	puts("accept");
	SetNonBlock(server.socket.sock_fd, 1);
	int clifd;
	while (1)
	{
	//	puts("accept");
//		memset(buf, 0, sizeof(buf));
		struct sockaddr_in cliaddr;
		socklen_t cliaddrlen = sizeof(struct sockaddr_in);
		clifd = accept(server.socket.sock_fd, (struct sockaddr*)&cliaddr, &cliaddrlen);
//		printf("%d\n", clifd);	
		if (clifd == -1)
		{
			if (errno != EAGAIN) perror("accept error:");
			break;
		}
		else 
		{
			Port[clifd] = ntohs(cliaddr.sin_port);
		//	printf("%d----%d\n", clifd, Port[clifd]);
	//		memset(buf, 0, sizeof(buf));
	//		string tmp = "";
	//		if (server.room_max ==0) tmp += "we have no room now, enter something to create the room";
	//		else tmp += "we have these room : ";
	//		for (int i = 0; i < server.room_max; ++i) 
	//		{
	//			tmp += DigToString(i);
	//			if (i < server.room_max - 1) tmp += ", ";
	//		}
	//		strcpy(buf, tmp.c_str());
		//	printf("%s\n", buf);
			server.epoll.Add(clifd, EPOLLIN | EPOLLET);
//			puts("jhe");			
		}
	}

	SetNonBlock(server.socket.sock_fd, 0);
}
void Read(Server& server, int fd, string &msg)
{
//	printf("000 %d\n", Port[fd]);
//	puts("reading");
//	printf("%d\n", Port[fd]);
//	Socket socket = Socket();
	memset(buf2, 0, sizeof(buf2));
//	printf("11 %d\n", Port[fd]);
	SetNonBlock(fd, 1);
	int cur = 0;
//	printf("%d %d\n", fd, Port[fd]);
	while (1)
	{
		int len = read(fd, buf2 + cur, 1024);
		
		if (len == 0)
		{
			puts("client close");
			if (Port[fd] == -1) break;
			int id = Room[Port[fd]];
			if (id == -1) break;
			for (int i = 0; i < server.room_max; ++i)
			{
				if (room_list[id][i] == fd)
				{
					room_list[id].erase(room_list[id].begin() + i);
					break;
				}
			}
			Room[Port[fd]] = -1;
			Port[fd] = 0;
			break;
		}
		if (len == -1)
		{
			if (errno == EAGAIN) break;
			exit(1);
		}
		cur += len;

	}
//	printf("%d %d```\n",fd, Port[fd]); 
	msg = string (buf2, cur);
//	cout << s.substr(16, cur - 16)  << endl;
//	puts("finish read");

//	printf("111 %d\n", Port[fd]);
//	memcpy(&node, msg, sizeof(msg));
/*	
//	memset(buf, 0, sizeof(buf));
	if (node.cmd_id == 2)
	{
//		puts("ok");
//		printf("%s\n", socket.buf);
		string tmp = "";
		tmp += "port: ";
		tmp += DigToString(Port[fd]);
//		printf("%d??%d\n", fd, Port[fd]);
//		printf("%s\n", DigToString(Port[fd]).c_str());
		tmp +=  " enter the room ";
//		strcpy(buf, tmp.c_str());
		int num = StringToDig(socket.buf);
//		printf("%d\n", num);
		if (num >= server.room_max) num = server.room_max++;
		Room[Port[fd]] = num;
		int id = num;
		room_list[id].push_back(fd);
		tmp += DigToString(id) + "";
//		printf("%s\n", tmp.c_str());
		strcpy(buf, tmp.c_str());
//		puts("pb");
		for (int i = 0; i < room_list[id].size(); ++i)
		{
			int fd = room_list[id][i];
	//		server.epoll.Add(fd, EPOLLOUT | EPOLLET);
			server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
		}

		SetNonBlock(fd, 0);
//		puts("11");
		return ;
	}
//	puts("2");
	string tmp = "";
	tmp += "port: ";
	tmp += DigToString(Port[fd]);
	tmp += "  say: ";
	tmp += socket.buf;
	strcpy(buf, tmp.c_str());
	int id = Room[Port[fd]];
	for (int i = 0; i < room_list[id].size(); ++i) 
	{
		int fd = room_list[id][i];
	//	server.epoll.Add(fd, EPOLLOUT | EPOLLET);
		server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
	}
*/
//	puts("ok");
	SetNonBlock(fd, 0);
}


void Write(Server& server, int fd, string &msg)
{
//	puts("writing");
//	printf("%d\n", Port[fd]);
//	Socket socket = Socket();
//	puts("ok");
		
//	strcpy(socket.buf, buf);
//	printf("%s\n", buf);
//	memset(buf2, 0, sizeof(buf2));
//	memcpy(buf2, &socket, sizeof(socket));
	
//	Socket fuck = Socket();
//	memcpy(&fuck, buf, sizeof(buf));
//	printf("%s\n", fuck.buf);
	memset(buf, 0, sizeof(buf));
//	strcpy(buf, msg.c_str());
	memcpy(buf, msg.c_str(), msg.length());

	SetNonBlock(fd, 1);
	int cur = 0;
//	while (1)
	{
		int len = write(fd, buf + cur, 10204);
	//	printf("len = %d\n", len);
		if (len == 0)
		{
			puts("client close");
			int id = Room[Port[fd]];
			for (int i = 0; i < room_list[id].size(); ++i)
			{
				if (room_list[id][i] == fd)
				{
					room_list[id].erase(room_list[id].begin() + i);
					break;
				}
			}
			Room[Port[fd]] = -1;
			Port[fd] = 0;
			//puts("?");
		}
		else if (len == -1)
		{
			if (errno != EAGAIN) exit(1);
//			if (errno == EAGAIN) break;
//			exit(1);
		}
		cur += len;
//		puts("/.....");
	}
	SetNonBlock(fd, 0);
	server.epoll.Modify(fd, EPOLLIN | EPOLLET);
//	puts("finish write");
//	printf("%d\n", Port[fd]);

}
int main(int argc, char **argv)
{
//	s = "hello";
//	int x;
//	EncodeMsg(1, s, msg);
//	DecodeMsg(msg, x, s);
//	printf("%d--\n", x);
	memset(Port, -1, sizeof(Port));
	memset(Room, -1, sizeof(Room));
	for (int i = 0; i < 777; ++i) room_list[i].clear();
	if (argc != 3)
	{
		puts("please input the ip address and the port");
		exit(-1);
	}
	Server server = Server();
	assert(server.socket.Create());	
	assert(server.socket.Bind(argv[1], StringToDig(argv[2])));
	if (!server.socket.Listen()) puts("error!!!!");
	server.epoll.Add(server.socket.sock_fd, EPOLLIN | EPOLLET);
//	puts("why2222");
	int cmd_id = 3;
	while (1)
	{
//		puts("wait1");
//		perror("wait");
		int ret = server.epoll.Wait();
	//	printf("%d---\n", Port[6]);
//		printf("ret = %d\n", ret);
		for (int i = 0; i < ret; ++i)
		{
	//		printf("%d===\n", Port[6]);
			int fd = server.epoll.GetFd(i);
	//		printf("%d=-=-=\n", Port[fd]);
			int events = server.epoll.GetEvents(i);
			if (fd == server.socket.sock_fd)
				handle_accept(server);
			else if (events & EPOLLIN)
			{
	//			printf("%d****%d\n", fd, Port[fd]);	
				Read(server, fd, s);
	//			printf("after read %d\n", Port[fd]);
		//		puts("?");
				DecodeMsg(s, cmd_id, msg);			
		//		puts("llll");
		//		printf("cmd = %d\n", cmd_id);
				if (cmd_id == 1)
				{
	//				printf("oooo %d  %d\n", fd, Port[fd]);
					s = "";
					s += "Create the new room, room number is : ";
					s += DigToString(server.room_max++);
					s += " , now you can start to chat";
					Room[Port[fd]] = server.room_max - 1;
		//			printf("%d  =\n", Room[Port[fd]]);
					int id = Room[Port[fd]];
					room_list[id].push_back(fd);
					server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
	//				printf("%d  fidy %d\n", fd, Port[fd]);

				}
				else if (cmd_id == 2)
				{
					s = "";
					if (msg == "-1")
					{
						
						s += "please choose a number to enter , we have these room : ";
						for (int i = 0; i < server.room_max; ++i)
						{
							s += DigToString(i);
							if (i != server.room_max - 1) s += " , ";
						}
						
						server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
					}
					else 
					{
						s += "port : ";
						s += DigToString(Port[fd]);
						s += " enter the room_";
						s += msg;
						Room[Port[fd]] = StringToDig(msg);
						int id = Room[Port[fd]];
						room_list[id].push_back(fd);
						for (int i = 0; i < room_list[id].size(); ++i)
						{
							int fd = room_list[id][i];
							server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
						}
					}
				
				}
				else 
				{
					s = "";
					s += "port : ";
			//		printf("%d, %d\n", fd, Port[fd]);
					s += DigToString(Port[fd]);
					s += " say: ";
					s += msg;
					int id = Room[Port[fd]];
					for (int i = 0; i < room_list[id].size(); ++i)
					{
						int fd = room_list[id][i];
						server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
					}

				}

			}
			else if (events & EPOLLOUT)
			{
				EncodeMsg(cmd_id, s, msg);
				Write(server, fd, msg);

			}
		}
//		puts(".");
	}
}


