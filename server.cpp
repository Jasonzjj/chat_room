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
	char userid[10];
	char pwd[10];
	char name[10];
//	struct sockaddr_in address;
};
map<string, int>userfd;//when login,mark down userid and fd;

map<int, Node> online; //when login or quit, update it;
map<string, Node> signup;// when sign up, save it;

char buf[103333];
char buf2[102333];
string s, msg;

int Port[N];
int Room[N];

int Hash(string s)
{
	int hash = 0;
	int mod = 13337;
	int key = 37;
	for (int i = 0; i < s.length(); ++i)
	{
		hash = hash * key + s[i];
	}
	hash %= mod;
	hash = (hash + mod) % mod;
	return hash;
}
bool DecodeMsg(const string &data, int &cmd_id, string &msg, Node &node)
{

	memset(buf, 0, sizeof(buf));
	
	memcpy(buf, data.c_str(), data.length());

	//for (int i = 0; i < data.length(); ++i) buf[i] = data[i];
	char tmp[10];
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, buf);
	cmd_id = StringToDig(tmp);
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, buf + 10);
	int crc32 = StringToDig(tmp);
	memset(tmp, 0, sizeof(tmp));
	strcpy(tmp, buf + 20);
	int length = StringToDig(tmp);
//	memset(node, 0, sizeof(Node));
	strcpy(node.userid, buf + 30);
	strcpy(node.pwd, buf + 40);
	strcpy(node.name, buf + 50);

	if (length) msg = string(buf + 60, length);
	if (Hash(msg) != crc32) return false;
	
	if (length != msg.length()) return false;
	
	return true;
}


bool EncodeMsg(const int cmd_id, const string &data, const Node &node, string &msg)
{

	memset(buf, 0, sizeof(buf));
	string ts;
	ts = DigToString(cmd_id);
	strcpy(buf, ts.c_str());
	int crc32 = Hash(data);
	
	int length = data.length();
	
	strcpy(buf + 10, DigToString(crc32).c_str());
	strcpy(buf + 20, DigToString(length).c_str());
	strcpy(buf + 30, node.userid);
	strcpy(buf + 40, node.pwd);
	strcpy(buf + 50, node.name);
	memcpy(buf + 60, data.c_str(), length);
	

	
	msg = string(buf, 60 + length);
	
	return true;
}


void handle_accept(Server& server)
{
	puts("accept");
	SetNonBlock(server.socket.sock_fd, 1);
	int clifd;
	while (1)
	{
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
			
			server.epoll.Add(clifd, EPOLLIN | EPOLLET);
			//add an modify sentence;
			server.epoll.Modify(clifd, EPOLLIN | EPOLLET);
		}
	}

	SetNonBlock(server.socket.sock_fd, 0);
}
void Read(Server& server, int fd, string &msg)
{
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
			puts("clinet close in errors");

			/////////////////////////////////////////////
			for (map<int, Node>::iterator iter = online.begin(); iter != online.end(); ++iter)
			{
				if (iter->first == fd)
				{
					online.erase(iter);
					break;
				}
			}
			return ;
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
	
	SetNonBlock(fd, 0);
}


void Write(Server& server, int fd, string &msg)
{
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
//	puts("ok");
	int cmd_id = 3;
	Node node;
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
			//	Node node;
				DecodeMsg(s, cmd_id, msg, node);			
				s = "";
		//		puts("llll");
		//		printf("cmd = %d\n", cmd_id);
				if (cmd_id)
				{
					strcpy(node.name, signup[node.userid].name);
				}
				if (cmd_id == 0)  //sign up
				{
					if (signup.find(node.userid) != signup.end()) 
					{
						s = "";
						s += "the userid is exist!";
						
						//exist;
					}
					else 
					{
						s = "";
						s += "sign up success~";
						signup[node.userid] = node;	
					}
					server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
				//	server.epoll.Delete(fd, EPOLLIN | EPOLLET);
					
				}
				else if (cmd_id == 1) //login
				{
					if (signup.find(node.userid) == signup.end() || strcmp(signup[node.userid].pwd, node.pwd))
					{
						s = "";
						s += "password error or the account doesn't exist";
						server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
					}
					else 
					{
						s = "";
						memset(buf, 0, sizeof(buf));
						online[fd] = node;
//						strcpy(online[fd].name, sign[node.userid].name);
						userfd[node.userid] = fd;
						int k = 0;
						cmd_id = 5;
						for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
						{
							strcpy(buf + 10 * k, it->second.userid);
							k++;
							strcpy(buf + 10 * k, it->second.name);
							k++;
						}
						s += buf;
						for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
						{
							server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
						}	
					}

				}
				else if (cmd_id == 2) // exit
				{
					
					s = "";
				
					for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
					{
						if (fd == it->first)
						{
							online.erase(it);
							break;
						}
					}
					cmd_id = 5;
					int k = 0;
					memset(buf, 0, sizeof(buf));
					for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
					{
						strcpy(buf + 10 * k, it->second.userid);
						k++;
						strcpy(buf + 10 * k, it->second.name);
						k++;
					}
					s += buf;
					for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
					{
						server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
					}
				
//					serer.epoll.Modify(fd, EPOLLOUT | EPOLLET);
				}
				else if (cmd_id == 3) // one to one
				{
//					printf("%s", msg.c_str());
//					continue;
					char tmp[10];
					strcpy(tmp, msg.c_str());
										
					s = "";
					
					s = msg.substr(10, msg.length() - 10);
					
					server.epoll.Modify(userfd[tmp], EPOLLOUT | EPOLLET);

				}
				else if (cmd_id == 4) // to all people
				{
					s = "";
					s += msg;
					for (map<int, Node>::iterator it = online.begin(); it != online.end(); ++it)
					{
						server.epoll.Modify(fd, EPOLLOUT | EPOLLET);
					}
					
				}

			}
			else if (events & EPOLLOUT)
			{
				
				EncodeMsg(cmd_id, s, node, msg);
				Write(server, fd, msg);

			}
		}
//		puts(".");
	}
}


