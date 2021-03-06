#include "Client.h"

char buf[103024];

Client::Client()
{
//	Socket tmp1 = Socket::Socket();
	socket = *(new Socket());
//	socket = new Socket::Socket();
	epoll = *(new Epoll());
//	room_id = -1;
}
Client::~Client()
{
}

struct Node
{

	int magic_num;
	int cmd_id;
	int crc32;
	int length;
	
	string msg;
}node;

string s;  

string msg;
int StringToDig(string s)
{
	int num = 0;
	for (int i = 0; i < s.length(); ++i)
	{
		if (s[i] < '0' || s[i] > '9') return -1;
		num = num * 10 + s[i] - '0';
	}
	return num;
}
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
	memcpy(buf, data.c_str(), data.length());
	//for (int i = 0; i < data.length(); ++i) buf[i] = data[i];
	int magic_num = *(int *)&buf[0];
	cmd_id = *(int *)&buf[4];
	
	int crc32 = *(int *)&buf[8];
	
	int len = *(int *)&buf[12];
	
	//msg = buf + 16;
	
	msg = string(buf + 16, len);
	
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

void myexit(char *s)
{
	perror(s);
	exit(errno);
}
void SetNonBlock(int fd, const bool flag)
{
	int opts = fcntl(fd, F_GETFL);
	if (opts < 0 ) return ;
	if (flag) opts |= O_NONBLOCK;		
	else opts &= (~O_NONBLOCK);

	fcntl(fd, F_SETFL, opts);
	
}

void Write(int fd, Client& client, string msg)
{
//	puts("writing");
	if (fd != STDOUT_FILENO) //socketfd
	{
	//	puts("jjejeej");
//		Socket socket2 = Socket();
//		strcpy(socket2.buf, buf);
		memset(buf, 0, sizeof(buf));
//		memcpy(buf, &socket2, sizeof(socket2));
//		strcpy(buf, msg.c_str());
		memcpy(buf, msg.c_str(), msg.length());
		SetNonBlock(client.socket.sock_fd, 1);
		int cur = 0;
//		while (1)
		{
			int len = write(client.socket.sock_fd, buf + cur, 1024);
			if (len == 0)
			{
				puts("server close");
				exit(0);
			}
			if (len == -1)
			{
				if (errno != EAGAIN) exit(0);
//				if (errno == EAGAIN) break;
//				myexit("write");
			}
			cur += len;
		}
		SetNonBlock(client.socket.sock_fd, 0);
		client.epoll.Modify(client.socket.sock_fd, EPOLLIN | EPOLLET);
	}
	else 
	{
		puts("sjfiowejfoijweoifjwo");
		int cmd_id;
		string s;
		DecodeMsg(msg, cmd_id, s);
		printf("%s\n", s.c_str());
		client.epoll.Modify(fd, EPOLLIN | EPOLLET);
	}
}
int Read(int fd, Client& client, string &msg)
{
//	puts("reading");
//	Socket socket2 = Socket();
	if (fd != STDIN_FILENO) //socketfd
	{
//		puts("reading");
		memset(buf, 0, sizeof(buf));

		SetNonBlock(client.socket.sock_fd, 1);
		int cur = 0;
		while (1)
		{
			int len = read(client.socket.sock_fd, buf + cur, 1024);
			if (len == 0)
			{
				puts("server close");
				exit(0);
			}
			if (len == -1)
			{
				if (errno == EAGAIN) break;
				myexit("read");
			}
			cur += len;
		}
		if (strcmp(buf, "exit\n") == 0) return 1;
	//	Socket tmp = Socket();
	//	memcpy(&tmp, buf, sizeof(tmp));
//		printf("%s\n", tmp.buf);
//		exit(0);
//		puts("????");	
//		msg = buf;
		msg = string (buf, cur);
		SetNonBlock(client.socket.sock_fd, 0);
		client.epoll.Modify(client.socket.sock_fd, EPOLLIN | EPOLLET);
	//	client.epoll.Modify(STDOUT_FILENO, EPOLLOUT | EPOLLET);
//		puts("last:");
	}
	else 
	{
		memset(buf, 0, sizeof(buf));
		int cur = 0;
		SetNonBlock(fd, 1);
		while (1)
		{
			int len = read(fd, buf + cur, 1024);
		
			if (len == -1)
			{
				if (errno == EAGAIN) break;
				myexit("read");
			}
			cur += len;
		}
		struct Node node;
//		string s = buf;
		string s = string(buf, cur);
		EncodeMsg(3, s, msg);
//		strcpy(client.socket.buf, buf);
//		msg = buf;
//		memcpy(&client.socket, sizeof(buf));	
		SetNonBlock(fd, 0);
		client.epoll.Modify(client.socket.sock_fd, EPOLLOUT | EPOLLET);
	}
	return 0;
}




int main(int argc, char **argv)
{
	if (argc != 3) 
	{
	
		puts("please input the ip address and the port");
		exit(-1);
	}
	Client client = Client();
	client.socket.Create();
	if (!client.socket.Connect(argv[1], StringToDig(argv[2]))) myexit("connect");
	puts("connect succcess");
//	client.epoll.Add(STDOUT_FILENO, EPOLLIN | EPOLLET);
	client.epoll.Add(STDIN_FILENO, EPOLLIN | EPOLLET);
	client.epoll.Add(client.socket.sock_fd, EPOLLIN | EPOLLET);
//	printf("%d\n", client.socket.sock_fd);

	while (1)
	{
		puts("please select the number to continue;");
		puts("1----Create a new chatting room and join in");
		puts("2----Join in a exist chatting room");
		//	puts("3----exit");
	
		int op;
		op = 1;
		scanf("%d", &op);
		
		if (op == 1)
		{
			s = "-1";
			EncodeMsg(1, s, msg);
		//	cout << msg.substr(16, msg.length() - 16) << endl;
		//	puts("?");
			Write(-1, client, msg);
		//	puts(".");
		}
		else if (op == 2)
		{   
		
			s = "-1";
			EncodeMsg(2, s, msg);
			Write(-1, client, msg);
			sleep(1);
			Read(-1, client, s);  //from server
			
			int cmd_id;
			
			DecodeMsg(s, cmd_id, msg);
		
			printf("--  %s\n", msg.c_str());
			puts("please select");
			char room_num[12];
			scanf("%s", room_num);
			EncodeMsg(2, room_num, msg);	
			Write(-1, client, msg);
		}
		else 
		{
			puts("error");
			continue;
		}
		
		int cmd_id;
		while (1)
		{
	//		puts("?");	
			int ret = client.epoll.Wait();
			int out = 0;
	//		printf("ret = %d\n", ret);
			for (int i = 0; i < ret; ++i)
			{
				int fd = client.epoll.GetFd(i);
			//	printf("%d\n", fd);
				if (client.epoll.GetEvents(i) & EPOLLIN)
				{
					if (Read(fd, client, s)) out = 1;
					DecodeMsg(s, cmd_id, msg);		
					if (fd != STDIN_FILENO)
					{
						printf("%s\n", msg.c_str());
					}
				}
				else if (client.epoll.GetEvents(i) & EPOLLOUT) 
				{
					EncodeMsg(3, msg, s);
					Write(fd, client, s);
					
				}
			}
	//		puts("123");
			if (out) break;
	
		}
	
	}
}
