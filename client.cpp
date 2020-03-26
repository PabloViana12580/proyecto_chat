#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <errno.h>
#include <json/json.h>
using std::cout;
using std::cin;
#define MAX 80
#define PORT 8080
#define SA struct sockaddr
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)

int connected;
int code;

//generalizacion para responder
void clientResponse(int fd, int code){
	char buff[MAX];
	int n = 0;
	while((buff[n++] = getchar()) != '\n');

	switch(code){
		case 0://mandar username

			json_object *jobj = json_object_new_object();
			json_object *jstring = json_object_new_string(username);
			json_object_object_add(jobj,"username", jstring);
			strcpy(data, json_object_to_json_string(jobj));

			json_object *jobj = json_object_new_object();
			json_object *jint = json_object_new_int(code);
			json_object *jstring = json_object_new_string(data);
			json_object_object_add(jobj,"code", jint);
			json_object_object_add(jobj,"data", jstring);
			strcpy(buff, json_object_to_json_string(jobj));
		break;
		case 3://obtener información de alguien / todos
			json_object *jobj = json_object_new_object();
			//json_object *jint = json_object_new_int(istringstream(buff));
			json_object *jstring = json_object_new_string(buff);
			json_object_object_add(jobj,"user", jstring);
			strcpy(data, json_object_to_json_string(jobj));

			json_object *jobj = json_object_new_object();
			json_object *jint = json_object_new_int(code);
			json_object *jstring = json_object_new_string(data);
			json_object_object_add(jobj,"code", jint);
			json_object_object_add(jobj,"data", jstring);
			strcpy(buff, json_object_to_json_string(jobj));
		break;
		case 4://mandar nuevo status

		break;
		case 1://mandar mensaje
		break;
		case 5://desconectarse
			json_object *jobj = json_object_new_object();
			json_object *jstring = json_object_new_string(buff);
			json_object_object_add(jobj,"userid", jstring);
			strcpy(data, json_object_to_json_string(jobj));

			json_object *jobj = json_object_new_object();
			json_object *jint = json_object_new_int(code);
			json_object *jstring = json_object_new_string(data);
			json_object_object_add(jobj,"code", jint);
			json_object_object_add(jobj,"data", jstring);
			strcpy(buff, json_object_to_json_string(jobj));
		break;
		default:
		break;
	}

	cout<<"To server: "<< buff<<"\n";

	write(fd,buff, sizeof(buff));
}

int parserFromServer(char buff[]){

	int code;
	/*
	json_object * jobj = json_tokener_parse(buff);
	json_object_object_foreach(jobj, key, val){

	if (strncmp(json_object_get_int(key),"code") == 0){
		code = val;
	}

	json_object_object_foreach(jobj, key, val){
	if (strncmp(json_object_get_int(key),"data") == 0){
		cout<<"From Server: "<<val<<"\n";
	}
	*/

	return code;
}

void connectionHandler(int sockfd)
{

	for (;;){
		char buff[MAX];
		read(sockfd,buff,MAX);
		json_object * jobj = json_tokener_parse(buff);
		//char* code = "code";
		json_object_object_foreach(jobj, key, val)
		{
			if(strcmp(key,"code") == 0)
			{
				code = json_object_get_int(val);
			}
		}

		switch(code)
		{
			case 100:
				message = parceGetString(data,"message");
				cout << "From server: "<< message;
				clientResponse(sockfd, 0);
			break;
			case 5:
			{
				std::string username;
				//Por alguna razon truena cuando son mas de 5 caracteres
				cout<<"nombre de usuario (5 letras):\n";
				getline(cin, username);
				//Creacion de elementos del json
				json_object *jobjResponse = json_object_new_object();
				//codigo 5 usado para pedir nombre de usuario
				json_object *jint = json_object_new_int(0);
				//Construimos el json en si
				const char* tmp = username.c_str();
				json_object *jstring = json_object_new_string(tmp);
				json_object_object_add(jobj,"code", jint);
				json_object_object_add(jobj,"username", jstring);
				//copiamos el json a buff
				strcpy(buff, json_object_to_json_string(jobj));
				//Escribimos en el socket
				write(sockfd, buff, strlen(buff));
				code = 0;
				break;
			}
			case 0:
				break;
		}

		int code = parserFromServer(buff);
		/*
		if(code == 500){
			connected=1;
			close(sockfd);
			return;
		} else if(code == 1) {
			cout<<"todos";
			write(sockfd,"\0",1);
		} else {
			clientResponse(sockfd);
		}*/
	}
}

int main()
{
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;
	connected = 1;
	while (1){
		// si no esta conectado, conectarse
		if (connected){
			//crear socket
			sockfd = socket(AF_INET, SOCK_STREAM,0);
			if( sockfd == -1) {
				cout << "Socket creation failed...\n";
				exit(0);
			}
			bzero(&servaddr, sizeof(servaddr));
			//configurar socket
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
			servaddr.sin_port = htons(PORT);
			//conectarse
			if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
				cout << "connection with the server failed...\n";
				cout << strerror(errno);
				exit(0);
			}
			cout << "server listening...\n";
			connected = 0;
		} else{
			connectionHandler(sockfd);
		}
	}
}
