#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <string>
#include <iostream>

using std::cout;
using std::string;

//Definimos variables globales del servidor
#define MAX 1024
#define PORT 7070
#define SA struct sockaddr
#define MAXCLIENTS 20
#define BACKLOG 10 //cuantas conexiones pendientes va a aguantar el queue

int clientnum = 0;
bool serverON = 1;


int managementServer(int fd)
{

}

//Funcion para desplegar errores y salir del programa
static void error(const char* s)
{
	perror(s);
	exit(EXIT_FAILURE);
}

//funcion que ejecutara cada thread, recibe el file descriptor que regresa accept()
void *conHandler(void *filedescriptor)
{
	//Parseamos el parametro de void a long para usar el file descriptor
	long connectfd = (long) filedescriptor;

	//aumentamos el numero de conexiones activas en el server
	clientnum++;

	while(1) {
		if(connectfd<0 || managementServer(connectfd) == 1)
		{
			clientnum = clientnum - 1;
			pthread_exit(0);
		}
	}

}


int main(int argc, char* argv[])
{
	//configuracion para el socket
	int sockfd, len;
	intptr_t connfd;
	struct sockaddr_in server, client;
	socklen_t sin_size;

	/*
	socket()

	- Crea un endpoint de comunicacion 
	- Regresa un file descriptor que guarda una referencia hacia el endpoint

	Si la creacion del endpoint no resulta exitosa, error
	*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        error("socket");
    }

    //configuracion puerto y direccion
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP

    /*
	bind()

	- Asigna una direccion especifica al socket

	Si no es posible asignar la direccion al socket, error
    */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        error("bind");
    }else{
    	cout << "Socket succefully created and binded\n";
    }

    /*
	listen()

	- hace al socket, un socket pasivo. refiriendose a que aceptara solicitudes de 
	conexion por medio de accept()
		
	Si no es posible marcar el socket, error
    */
    if (listen(sockfd, BACKLOG) == -1)
    {
    	error("listen");
    }else{
    	cout << "Server listening ...\n";
    }

    //Ciclo infinito mientras el server este encendido para aceptar conexiones
    while(serverON) {
    	sin_size = sizeof(struct sockaddr_in);

    	/*
		accept()

		- Acepta la primera solicitud de conexion del queue de sockfd 
		- crea un nuevo socket conectado y regresa un file descriptor de ese nuevo socket, 
		el socket original no se ve afectado por esta llamada
		*/
    	if(connfd = accept(sockfd, (struct sockaddr *)&client, &sin_size) < 0)
    	{
    		error("accept");
    	}

    	cout << "Server detected new connection\n";

    	//Utilizamos un thread para manejar cada una de las conexiones al socket
    	pthread_t conection_thread;
    	pthread_create(&conection_thread, NULL, conHandler, (void *)connfd);

    }
	



}
