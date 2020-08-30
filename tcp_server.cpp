#include "tcp_server.h"

tcp_server* tcp_server::server_ptr = NULL;
tcp_server::tcp_server()
{}

tcp_server::tcp_server(const tcp_server&)
{}

bool tcp_server::server_listen()
{
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd < 0) {
		perror("socket");
		return false;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(8851);
	int ret = bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr));
	if (ret < 0) {
		perror("bind");
		return false;
	}
	ret = listen(listen_fd,128);
	if (ret < 0) {
		perror("listen");
		return false;
	}
	//创建epoll树
	epoll_tree = epoll_create(1);
	//上epoll树
	struct epoll_event evt;
	evt.data.fd = listen_fd;
	evt.events = EPOLLIN;
	ret = epoll_ctl(epoll_tree, EPOLL_CTL_ADD, listen_fd, &evt);
	if (ret == -1)
	{
		perror("epoll_ctl");
		std::cout << "listen fd:" << listen_fd << std::endl;
		std::cout << "epoll_tree fd:" << epoll_tree << std::endl;
		return false;
	}
	std::cout << "established listen and create epoll tree" << std::endl;
	return true;
}

void tcp_server::server_run()
{
	if (!server_listen())
	{
		std::cout << "server listen error" << std::endl;
		server_close();
		return;
	}
	int size = sizeof(epr) / sizeof(epoll_event);
	//将所有主动连接的socket都记录并监听
	std::cout << "main server loop start" << std::endl;
	while(1) {
		epoll_count = epoll_wait(epoll_tree, epr, size, -1);
		for (int i = 0; i < epoll_count; i++) {
			if (epr[i].data.fd == listen_fd) { //判断是不是监听文件,如果监听文件有变化则将对应的通信文件添加到epoll树和vector
				int cfd = accept(listen_fd, NULL, NULL);
				if (cfd < 0){
					perror("accept");
					continue;
				}
				epoll_event evt;
				evt.data.fd = cfd;
				evt.events = EPOLLIN;
				int ret = epoll_ctl(epoll_tree, EPOLL_CTL_ADD, cfd, &evt);
				if (ret == -1)
				{
					perror("epoll_ctl");
					return ;
				}
				accept_list.push_back(cfd);
				std::cout << "Add a link file" << std::endl;
			}
			else{
				//获取消息并加入消息队列或转发
				char buf[100] = { 0 };
				int count = recv(epr[i].data.fd, buf, sizeof(buf), 0);
				if (count <= 0 ){
					perror("recv");
					continue;
				}
				for (auto it : accept_list){
					//转发数据
					if (it != epr[i].data.fd){
						send(it, buf, count, 0);
						std::cout << "send data: " << buf << " file describes: " << it << std::endl;
					}
				}
			}
		}
	}
}

void tcp_server::server_close()
{
	for (auto it : accept_list){
		close(it);
	}
	close(listen_fd);
	close(epoll_tree);
}

tcp_server* tcp_server::getinstance()
{
	if (server_ptr != nullptr){
		return server_ptr;
	}
	server_ptr = new tcp_server;
	return server_ptr;
}


