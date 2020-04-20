#include <iostream>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <stdio.h>
#include <errno.h>
#include <string.h> 


// #include <json/json.h>

#include "mensaje.pb.h"
using namespace chat;
using namespace std;

using std::cout;
using std::endl;
using std::string;
using std::cin;
using namespace std;
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

int parserFromServer(string buffer)
{	
	ServerMessage s;
	s.ParseFromString(buffer);
	switch(s.option()){
		case 1: // broadcast
			cout << "Mensaje al grupo enviado por: \t" << s.broadcast().userid() << endl;
			cout << "Mensaje: \t" << s.broadcast().message() << endl;
			cout << "\n" << endl;
			return 1;
		case 2: // directmessage
			cout << "Mensaje Privado enviado por: \t" << s.message().userid() <<endl;
			cout << "Mensaje: \t" << s.message().message() << endl;
			cout << "\n" << endl;
			return 1;
		case 3: // error
			cout << "Recibiendo Error" << endl;
			cout << "ERROR: \t" << s.error().errormessage() << endl;
			cout << "\n" << endl;
			return 0;
		case 4: // myInfoResponse
			cout << "MY INFO RESPONSE RECIVIDO" << endl;
			cout << "ID: \t" << s.myinforesponse().userid() << endl;
			cout << "\n" << endl;
			return 0;
		case 5: // connectedUserResponse
			cout << "Recibiendo connectedUserResponse" << endl;
			cout << "Cantidad de usuarios:\t" << s.connecteduserresponse().connectedusers_size() << endl
			for (int j = 0; j < s.connecteduserresponse().connectedusers_size(); j++){
				cout << "UserID: " << s.connecteduserresponse().connectedusers(j).userid() << endl;
				cout << "Username: " << s.connecteduserresponse().connectedusers(j).username() << endl;
			}
			cout << "\n" << endl;
			return 0;
		case 6: // changeSatatusResponse
			cout << "Recibiendo ChangeStatusResponse: \t" << endl;
			cout << "ID: \t" << s.changestatusresponse().userid() << endl;
			cout << "Status: \t "<< s.changestatusresponse().status() << endl;
			cout << "\n" << endl;
			return 0;
		case 7: // broadcastRespnse (sent message status)
			cout << "Recibiendo BroadcastResponse \t" << endl;
			cout << "Mesage Status: \t" << s.broadcastresponse().messagestatus();
			cout << "\n" << endl;

			return 0;
		case 8: // directMessageResponse (sent message status)
			cout << "Recibiendo DirectMessageResponse \t" << endl;
			cout << "Mesage Status: \t" << s.directmessageresponse().messagestatus() << endl;
			cout << "\n" << endl;
			return 0;
	}
	
}


int main(int argc, char *argv[])
{
	int fd;
	int numbytes;
	char buf[MAX_];
	struct hostent *he;
	struct sockaddr_in server;
	
	if ((he = gethostbyname(argv[1])) == NULL) {
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
	cout << "\nINICIA THREE WAY HANDSHAKE" << endl;
	// MY INFO REQ
	// Se crea instacia tipo MyInfoSynchronize y se setean los valores deseables
    MyInfoSynchronize * mySinc(new MyInfoSynchronize);
    mySinc->set_username(argv[2]);
    mySinc->set_ip("127.0.0.1");
	// Para enviar un mensaje
    // Se crea instancia de Mensaje, se setea los valores deseados
    ClientMessage m;
    m.set_option(1);
    m.set_allocated_synchronize(mySinc);
    string msg;
	if(m.has_option()){
		m.SerializeToString(&msg);
		sprintf(buf,"%s",msg.c_str());
		// Se envia el mensaje
		send(fd , buf , sizeof(buf) , 0 );
		cout << "Se envio el MY INFO REQ." << endl;
	}
    
	string data;
	numbytes = -1;
	// Para recibir un mensaje
	cout << "Esperando MY INFO RESPONSE DEL SERVIDOR" << endl;
	while(numbytes==-1){
		numbytes = recv(fd, buf, MAX_, 0);
		buf[numbytes] = '\0';
		data = buf;
	}
	
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
	cout << "Se envio el MY INFO ACK." << endl;
	cout << "TERMINA THREE WAY HANDSHAKE.\n" << endl;
	// Finaliza el 3w handshake


	// inicializacion de instancias
	connectedUserRequest * myUsersRequest(new connectedUserRequest);
	ChangeStatusRequest * myChangeStatus(new ChangeStatusRequest);
	BroadcastRequest * myBroadcast(new BroadcastRequest);
	DirectMessageRequest * myMessage(new DirectMessageRequest);

	char charInput;
	int intInput;
	int opcion;
	int esResponse;
	while(1){
		cout << "\nQue quieres hacer?" << endl;
		cout << "(1) Obtener usuarios conectados" << endl;
		cout << "(2) Cambiar estado de conexion" << endl;
		cout << "(3) Transmitir un mensaje a todos los usuarios" << endl;
		cout << "(4) Enviar un mensaje directo" << endl;
		cout << "(5) Salir\t" << endl;
		cout << "Escriba opción" << endl;
		fflush( stdin );
		scanf("%d",&opcion);
		if (opcion == 1 ){ // connectedUserRequest
			myUsersRequest->set_userid(0);
			myUsersRequest->set_username(argv[2]);
			m.set_option(2);
			m.set_allocated_connectedusers(myUsersRequest);
			msg = "";
			m.SerializeToString(&msg);
			sprintf(buf,"%s",msg.c_str());
			send(fd, buf, sizeof(buf), 0);
			cout << "Se envio el connectedUserRequest" << endl;
			cout << "Esperando respuesta del servidor \n" << endl;
			esResponse = 1;
			while(esResponse == 1)
			{
				numbytes = -1;
				while(numbytes==-1){
					numbytes = recv(fd, buf, MAX_, 0);
					buf[numbytes] = '\0';
					data = buf;
				}
				esResponse = parserFromServer(data);
			}
			usleep(2500000);
			continue;
		} else if (opcion == 2 ){ // changeStatus
			cout << "Ingrese su nuevo estado." << endl;
			fflush( stdin );
            scanf( "%c", &charInput );
			std::string s(sizeof(charInput), charInput);
			myChangeStatus->set_status(s);
			m.set_option(3);
			m.set_allocated_changestatus(myChangeStatus);
			msg = "";
			m.SerializeToString(&msg);
			sprintf(buf,"%s",msg.c_str());
			send(fd, buf, sizeof(buf), 0);
			cout << "Se envio el ChangeStatusRequest" << endl;
			cout << "Esperando respuesta del servidor \n" << endl;
			esResponse = 1;
			while(esResponse == 1)
			{
				numbytes = -1;
				while(numbytes==-1){
					numbytes = recv(fd, buf, MAX_, 0);
					buf[numbytes] = '\0';
					data = buf;
				}
				esResponse = parserFromServer(data);
			}
			usleep(2500000);
			continue;
		} else if (opcion == 3 ){ //broadcast
			cout << "Ingrese el mensaje que desea enviar" << endl;
			fflush( stdin );
            scanf( "%c", &charInput );
			std::string s(sizeof(charInput), charInput);
			myBroadcast->set_message(s);
			m.set_option(4);
			m.set_allocated_broadcast(myBroadcast);
			msg = "";
			m.SerializeToString(&msg);
			sprintf(buf,"%s",msg.c_str());
			send(fd, buf, sizeof(buf), 0);
			cout << "Se envio el BroadcastRequest" << endl;
			cout << "Esperando respuesta del servidor \n" << endl;
			esResponse = 1;
			while(esResponse == 1)
			{
				numbytes = -1;
				while(numbytes==-1){
					numbytes = recv(fd, buf, MAX_, 0);
					buf[numbytes] = '\0';
					data = buf;
				}
				esResponse = parserFromServer(data);
			}
			usleep(2500000);
			continue;
		} else if (opcion == 4 ){ //directmessage
			cout << "Ingrese el id del usuario al que le quiere enviar un mensaje" << endl;
			fflosh( stdin );
			scanf("%d", &intInput);
			cout << "Ingrese el mensaje que desea enviar" << endl;
			fflush( stdin );
            scanf( "%c", &charInput );
			std::string s(sizeof(charInput), charInput);
			myMessage->set_message(s);
			myMessage->set_username(USER);
			myMessage->set_userid(intInput)
			m.set_option(5);
			m.set_allocated_directmessage(myMessage);
			msg = "";
			m.SerializeToString(&msg);
			sprintf(buf,"%s",msg.c_str());
			send(fd, buf, sizeof(buf), 0);
			cout << "Se envio el DirectMessageRequest" << endl;
			cout << "Esperando respuesta del servidor \n" << endl;
			esResponse = 1;
			while(esResponse == 1)
			{
				numbytes = -1;
				while(numbytes==-1){
					numbytes = recv(fd, buf, MAX_, 0);
					buf[numbytes] = '\0';
					data = buf;
				}
				esResponse = parserFromServer(data);
			}
			usleep(2500000);
			continue;
		} else if (opcion == 5 ){
			cout << "HASTA LA VISTA BBY" << endl;
			break;
		} else if (opcion < 0 || opcion >5){
			cout << "Opcion incorrecta, prueba de nuevo." << endl;
			continue;
		}
				
	}

	// finalizacion del cliente
	google::protobuf::ShutdownProtobufLibrary();
    return 1;
	

}



// //generalizacion para responder
// void clientResponse(int fd, int code){
// 	char buff[MAX];
// 	int n = 0;
// 	while((buff[n++] = getchar()) != "\n");

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
