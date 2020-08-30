#include <cstdio>
#include "tcp_server.h"
int main()
{
    tcp_server* tcp = tcp_server::getinstance();
    tcp->server_run();
    return 0;
}