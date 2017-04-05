objects = Client.o Epoll.o Socket.o
objects2 = server.o Epoll.o Socket.o

all : client2 server2

client2 : $(objects)
	g++ -o client2 $(objects)

server2 : $(objects2)
	g++ -o server2 $(objects2)

server.o : Socket.h server.h Epoll.h
Client.o : Socket.h Client.h Epoll.h
Epoll.o : Socket.h Epoll.h
Socket.o : Socket.h


.PHONY : clean

clean:
	-rm client2 $(objects)
	-rm server2 $(objects2)
