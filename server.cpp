#include <iostream>
#include <stdio.h>
using std::cout;
using std::string;
#define MAX 1024
#define PORT 7070
#define SA struct sockaddr
#define MAXCLIENTS 20

int main(int argc, char* argv[])
{
	//configuracion para el socket
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	if(sockfd == -1)
	{
		cout << "Socket creation fail!\n";
		exit(0);
	}

	

}
