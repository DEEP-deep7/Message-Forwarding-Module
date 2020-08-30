#pragma once
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <list>
#include <sys/epoll.h>

class tcp_server
{
public:
	bool server_listen();
	void server_run();
	void server_close();
	static tcp_server* getinstance();
private:
	struct sockaddr_in addr;
	int listen_fd;
	int connect_fd;
	std::list<int> accept_list;
	struct epoll_event epr[100];
	int epoll_tree;
	int epoll_count;
	tcp_server();
	tcp_server(const tcp_server&);
	static tcp_server* server_ptr;
};
