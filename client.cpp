#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <errno.h>
// #include <json/json.h>

#include "mensaje.pb.h"
using namespace chat;

using std::cout;
using std::endl;
using std::string;
using std::cin;
#define MAX_ 16384
#define PORT 7070
#define HOSTNAME "192.168.1.6"
#define SA struct sockaddr
#define bzero(b,len) (memset((b), '\0', (len)), (void) 0)
#define USER "oliversinn"


int connected;
int code;
int32_t opcion;

// Funcion para errores
static void err(const char* s) {
    perror(s);
    exit(EXIT_FAILURE);
}

void parserFromServer(string buffer)
{	
	ServerMessage s;
	s.ParseFromString(buffer);
	switch(s.option()){
		case 1: // broadcast
			cout << "'Mensaje al Grupo: \t'" << endl;
			cout << "'Enviado por: \t'" << s.broadcast().userid() << endl;
			cout << "'Mensaje: \t'" << s.broadcast().message() << endl;
			break;
		case 2: // directmessage
			cout << "'Mensaje Privado: \t'" << endl;
			cout << "'Enviado por: \t'" << s.message().userid() <<endl;
			cout << "'Mensaje: \t'" << s.message().message() << endl;
			break;
		case 3: // error
			cout << "'Recibiendo Error'" << endl;
			cout << "'Servidor:\t'" << endl;
			cout << "'ERROR: \t'" << s.error().errormessage() << endl;
			break;
		case 4: // myInfoResponse
			cout << "'Recibiendo MY INFO RESP.'" << endl;
			cout << "'Servidor:\t'" << endl;
			cout << "ID: \t" << s.myinforesponse().userid() << endl;
			break;
		case 5: // connectedUserResponse

			break;
		case 6: // changeSatatusResponse
			cout << "'Recibiendo ChangeStatusResponse: \t'" << endl;
			cout << "'Servidor: \t'" << endl;
			cout << "'ID: \t'" << s.changestatusresponse().userid() << endl;
			cout << "'Status: \t' "<< s.changestatusresponse().status() << endl;
			break;
		case 7: // broadcastRespnse (sent message status)
			cout << "'Recibiendo BroadcastResponse \t'" << endl;
			cout << "'Servidor: \t'" << endl;
			cout << "'Mesage Status: \t'" << s.broadcastresponse().messagestatus();
			break;
		case 8: // directMessageResponse (sent message status)
			cout << "'Recibiendo DirectMessageResponse \t'" << endl;
			cout << "'Servidor:\t'" << endl;
			cout << "Mesage Status: \t" << s.directmessageresponse().messagestatus() << endl;
			break;
	}
	
}


int main(int argc, char *argv[])
{
	int fd;
	int numbytes;
	char buf[MAX_];
	struct hostent *he;
	struct sockaddr_in server;
	
	if ((he = gethostbyname(HOSTNAME)) == NULL) {
		err("gethostbyname");
	}

	if ((fd = socket(AF_INET, SOCK_STREAM,0)) == -1) {
		err("socket creation failed");
	}

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *((struct in_addr *)he->h_addr);

	if (connect(fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        err("connection with the server failed...");
    }
    
	// Inicia el three way handshake
	cout << "'Inicia el 3w handshake'" << endl;
	// MY INFO REQ
	// Se crea instacia tipo MyInfoSynchronize y se setean los valores deseables
    MyInfoSynchronize * mySinc(new MyInfoSynchronize);
    mySinc->set_username(USER);
    mySinc->set_ip("127.0.0.1");
	// Para enviar un mensaje
    // Se crea instancia de Mensaje, se setea los valores deseados
	opcion = 1;
    ClientMessage m;
    //m.set_option(opcion);
    m.set_allocated_synchronize(mySinc);
	cout << "Has option: \t" << m.has_option() << endl;
    string msg;
	if(m.has_option()){
		m.SerializeToString(&msg);
		sprintf(buf,"%s",msg.c_str());
		// Se envia el mensaje
		send(fd , buf , sizeof(buf) , 0 );
		cout << "Se envio el MY INFO REQ." << endl;
	}
    

	// Para recibir un mensaje
	numbytes = recv(fd, buf, MAX_, 0);
	buf[numbytes] = '\0';
	string data = buf;
	// Se parcea la respuesta esperando que sea el MyInfoResponse
	parserFromServer(data);

	// Se manda el MY INFO ACK.
	MyInfoAcknowledge * myAck(new MyInfoAcknowledge);
	m.set_option(6);
 	m.set_allocated_acknowledge(myAck);
 	msg = "";
	m.SerializeToString(&msg);
	sprintf(buf,"%s",msg.c_str());
	// Se envia el mensaje
    send(fd , buf , sizeof(buf) , 0 );
	cout << "'Se envio el MY INFO ACK.'" << endl;
	cout << "'Se termino el 3w handshake'" << endl;
	// Finaliza el 3w handshake


	// Inicia el envio de un mensaje
	DirectMessageRequest * myMessage(new DirectMessageRequest);
	myMessage->set_message("Mensaje directo de prueba");
	myMessage->set_userid(1); // supongo que este id es generado por el server pero le voy a poner 1 hardkodeado
	myMessage->set_username(USER); // no se si se pone el user al que le quiero enviar el mensaje o el mio
	m.set_option(5); // option 5: directMessage
	m.set_allocated_directmessage(myMessage);
	msg = "";
	m.SerializeToString(&msg);
	sprintf(buf,"%s",msg.c_str());
	send(fd, buf, sizeof(buf), 0);
	cout << "'Se envio el DirectMessageRequest'" << endl;

	numbytes = recv(fd, buf, MAX_, 0);
	buf[numbytes] = '\0';
	data = buf;
	// Se parcea la respuesta esperando que sea un direct message response
	parserFromServer(data);

	// finalizacion del cliente
	google::protobuf::ShutdownProtobufLibrary();
    return 1;
	

}



// //generalizacion para responder
// void clientResponse(int fd, int code){
// 	char buff[MAX];
// 	int n = 0;
// 	while((buff[n++] = getchar()) != '\n');

// 	switch(code){
// 		case 0://mandar username

// 			json_object *jobj = json_object_new_object();
// 			json_object *jstring = json_object_new_string(username);
// 			json_object_object_add(jobj,"username", jstring);
// 			strcpy(data, json_object_to_json_string(jobj));

// 			json_object *jobj = json_object_new_object();
// 			json_object *jint = json_object_new_int(code);
// 			json_object *jstring = json_object_new_string(data);
// 			json_object_object_add(jobj,"code", jint);
// 			json_object_object_add(jobj,"data", jstring);
// 			strcpy(buff, json_object_to_json_string(jobj));
// 		break;
// 		case 3://obtener información de alguien / todos
// 			json_object *jobj = json_object_new_object();
// 			//json_object *jint = json_object_new_int(istringstream(buff));
// 			json_object *jstring = json_object_new_string(buff);
// 			json_object_object_add(jobj,"user", jstring);
// 			strcpy(data, json_object_to_json_string(jobj));

// 			json_object *jobj = json_object_new_object();
// 			json_object *jint = json_object_new_int(code);
// 			json_object *jstring = json_object_new_string(data);
// 			json_object_object_add(jobj,"code", jint);
// 			json_object_object_add(jobj,"data", jstring);
// 			strcpy(buff, json_object_to_json_string(jobj));
// 		break;
// 		case 4://mandar nuevo status

// 		break;
// 		case 1://mandar mensaje
// 		break;
// 		case 5://desconectarse
// 			json_object *jobj = json_object_new_object();
// 			json_object *jstring = json_object_new_string(buff);
// 			json_object_object_add(jobj,"userid", jstring);
// 			strcpy(data, json_object_to_json_string(jobj));

// 			json_object *jobj = json_object_new_object();
// 			json_object *jint = json_object_new_int(code);
// 			json_object *jstring = json_object_new_string(data);
// 			json_object_object_add(jobj,"code", jint);
// 			json_object_object_add(jobj,"data", jstring);
// 			strcpy(buff, json_object_to_json_string(jobj));
// 		break;
// 		default:
// 		break;
// 	}

// 	cout<<"To server: "<< buff<<"\n";

// 	write(fd,buff, sizeof(buff));
// }

// int parserFromServer(char buff[]){

// 	int code;
	
// 	json_object * jobj = json_tokener_parse(buff);
// 	json_object_object_foreach(jobj, key, val){

// 	if (strncmp(json_object_get_int(key),"code") == 0){
// 		code = val;
// 	}

// 	json_object_object_foreach(jobj, key, val){
// 	if (strncmp(json_object_get_int(key),"data") == 0){
// 		cout<<"From Server: "<<val<<"\n";
// 	}


// 	return code;
// }

// void connectionHandler(int sockfd)
// {

// 	for (;;){
// 		char buff[MAX];
// 		read(sockfd,buff,MAX);
// 		json_object * jobj = json_tokener_parse(buff);
// 		//char* code = "code";
// 		json_object_object_foreach(jobj, key, val)
// 		{
// 			if(strcmp(key,"code") == 0)
// 			{
// 				code = json_object_get_int(val);
// 			}
// 		}

// 		switch(code)
// 		{
// 			case 100:
// 				message = parceGetString(data,"message");
// 				cout << "From server: "<< message;
// 				clientResponse(sockfd, 0);
// 			break;
// 			case 5:
// 			{
// 				std::string username;
// 				//Por alguna razon truena cuando son mas de 5 caracteres
// 				cout<<"nombre de usuario (5 letras):\n";
// 				getline(cin, username);
// 				//Creacion de elementos del json
// 				json_object *jobjResponse = json_object_new_object();
// 				//codigo 5 usado para pedir nombre de usuario
// 				json_object *jint = json_object_new_int(0);
// 				//Construimos el json en si
// 				const char* tmp = username.c_str();
// 				json_object *jstring = json_object_new_string(tmp);
// 				json_object_object_add(jobj,"code", jint);
// 				json_object_object_add(jobj,"username", jstring);
// 				//copiamos el json a buff
// 				strcpy(buff, json_object_to_json_string(jobj));
// 				//Escribimos en el socket
// 				write(sockfd, buff, strlen(buff));
// 				code = 0;
// 				break;
// 			}
// 			case 0:
// 				break;
// 		}

// 		int code = parserFromServer(buff);
// 		/*
// 		if(code == 500){
// 			connected=1;
// 			close(sockfd);
// 			return;
// 		} else if(code == 1) {
// 			cout<<"todos";
// 			write(sockfd,"\0",1);
// 		} else {
// 			clientResponse(sockfd);
// 		}*/
// 	}
// }

// int main()
// {
// 	int sockfd, connfd;
// 	struct sockaddr_in servaddr, cli;
// 	connected = 1;
// 	while (1){
// 		// si no esta conectado, conectarse
// 		if (connected){
// 			//crear socket
// 			sockfd = socket(AF_INET, SOCK_STREAM,0);
// 			if( sockfd == -1) {
// 				cout << "Socket creation failed...\n";
// 				exit(0);
// 			}
// 			bzero(&servaddr, sizeof(servaddr));
// 			//configurar socket
// 			servaddr.sin_family = AF_INET;
// 			servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
// 			servaddr.sin_port = htons(PORT);
// 			//conectarse
// 			if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
// 				cout << "connection with the server failed...\n";
// 				cout << strerror(errno);
// 				exit(0);
// 			}
// 			cout << "server listening...\n";
// 			connected = 0;
// 		} else{
// 			connectionHandler(sockfd);
// 		}
// 	}
// }
