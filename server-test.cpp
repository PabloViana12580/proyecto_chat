/*
 * Description:  test protobuf in server
 *
 * Copyright (C) 2011 lytsing.org
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// refer to: http://hideto.iteye.com/blog/445848

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

#include "mensaje.pb.h"

#define PORT 7070           // the port users will be connecting to
#define MAXDATASIZE 16384    // max number of bytes we can send at once
#define BACKLOG 10          // how many pending connections queue will hold
#define MAX_CLIENTS 20

using std::cout;
using std::endl;
using std::string;
using namespace chat;


void sigchld_handler(int s) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int clientnum = 0;
bool serverON = 1;

struct ClientInformation
{ 
    int id;
    int fd;
    int status; //0 activo, 1 ocupado, 2 desconectado
    int last_connected;
    string username;
    ClientInformation():id(-1),fd(-1),status(-1),last_connected(1000){}
}current_clients[MAX_CLIENTS];

int checkUser(int fd, string username)
{
    char buffer[MAXDATASIZE];
    if (clientnum > MAX_CLIENTS){ // si aun hay espacio
        /*
            MyInfoResponse * MyInfo(new MyInfoResponse);
            MyInfo -> set_userid(fd);

            ServerMessage sm;
            sm.set_option(4);
            sm.set_allocated_myinforesponse(MyInfo);
            string msg;
            sm.SerializeToString(&msg);
            sprintf(buffer,"%s",msg.c_str());
            send(fd , buffer , sizeof(buffer) , 0 );
            cout << "Se envio el MY INFO RESPONSE" << endl;
        */
        ErrorResponse * MyError(new ErrorResponse);
        MyError -> set_errormessage("Servidor lleno");

        ServerMessage sm;
        sm.set_option(3);
        sm.set_allocated_error(MyError);
        string msg;
        sm.SerializeToString(&msg);
        sprintf(buffer,"%s",msg.c_str());
        send(fd , buffer , sizeof(buffer) , 0 );
        cout << "Se envio ERROR RESPONSE" << endl;
        //er.set_option(1);
        return 0;
    }

    int i;
    for(i=0;i<MAX_CLIENTS;i++){
        if (current_clients[i].fd != -1 &&
            username.compare(current_clients[i].username) == 0
        ){
            ErrorResponse * MyError(new ErrorResponse);
            MyError -> set_errormessage("Nombre de usuario ya existe");

            ServerMessage sm;
            sm.set_option(3);
            sm.set_allocated_error(MyError);
            string msg;
            sm.SerializeToString(&msg);
            sprintf(buffer,"%s",msg.c_str());
            send(fd , buffer , sizeof(buffer) , 0 );
            cout << "Se envio ERROR RESPONSE" << endl;
            return 0;
        }
    }

    for(i=0;i<MAX_CLIENTS;i++){
        cout<<"it:"<<i<<"\n";
        cout<<"fd:"<<current_clients[i].fd<<"\n";
        if(current_clients[i].fd == -1){
            current_clients[i].id = i;
            current_clients[i].fd=fd;
            current_clients[i].username=username;
            current_clients[i].status = 0;
            current_clients[i].last_connected = 0;
            
            //char userInfo[MAX];
            //strcpy(userInfo,json_object_to_json_string(jobj));
            //strcpy(buff,createJson("user",userInfo).c_str());
            cout<<"1:"<<current_clients[0].username<<"\n";
            cout<<"2:"<<current_clients[1].username<<"\n";
            cout<<"3:"<<current_clients[2].username<<"\n";

            return 1;   
        }
    }
    return 1;
}

//Recibe clientMessage y dependiendo de la opcion discienre que accion del server ejecutar
int managementServer(int fd)
{
    int numbytes,action,code;
    char buf[MAXDATASIZE];
    char buffer[MAXDATASIZE];
    numbytes = recv(fd, buf, MAXDATASIZE, 0);
    buf[numbytes] = '\0';
    string a = buf;
        // cout << "Client Message: " << a << endl;

        // Receive  msg from clients
        // demo::People p;
        // p.ParseFromString(a);
        // cout << "People:\t" << endl;
        // cout << "Name:\t" << p.name() << endl;
        // cout << "ID:\t" << p.id() << endl;
        // cout << "Email:\t" << p.email() << endl;

    ClientMessage c;
    c.ParseFromString(a);
    code = c.option();

    cout<<"code: "<<code<<"\n";

    switch(code)
    {
        case 1: //conecction handshake "synchronize"
            cout << "Se detecta opcion numero 1 synchronize\n";
            action = 0;
    }

    cout<<"action: "<<action<<"\n";

    switch(action)
    {
        case 0:
        {
            int resp = checkUser(fd,c.synchronize().username());
            if(resp != 0){
                MyInfoResponse * MyInfo(new MyInfoResponse);
                MyInfo -> set_userid(fd);

                ServerMessage sm;
                sm.set_option(4);
                sm.set_allocated_myinforesponse(MyInfo);
                string msg;
                sm.SerializeToString(&msg);
                sprintf(buffer,"%s",msg.c_str());
                send(fd , buffer , sizeof(buffer) , 0 );
                cout << "Se envio el MY INFO RESPONSE" << endl;
            }
            //Iniciando conexion
                    // // Send msg to clients
        // string data;
        // demo::People to;
        // to.set_name("Lysting Huang");
        // to.set_id(123);
        // to.set_email("lytsing@hotmail.com");
        // to.SerializeToString(&data);
        // char bts[data.length()];
        // sprintf(bts, "%s", data.c_str());
        // send(connectfd, bts, sizeof(bts), 0);

        }
    }
}



//funcion que ejecutara cada thread, recibe el file descriptor que regresa accept()
void *conHandler(void *filedescriptor)
{
    //cout << 'aca toy' << endl;
    //Parseamos el parametro de void a long para usar el file descriptor
    long connectfd = (long) filedescriptor;

    //aumentamos el numero de conexiones activas en el server
    clientnum++;

    while(1) {
        if(connectfd<0 || managementServer(connectfd) == 1)
        {
            clientnum = clientnum - 1;
            close(connectfd);
            serverON = 0;
            pthread_exit(0);
        }
    }

}


//Funcion para desplegar errores y salir del programa
static void err(const char* s) {
    perror(s);
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
    int listenfd;
    int connectfd;
    struct sockaddr_in server;
    struct sockaddr_in client;
    socklen_t sin_size;
    struct sigaction sa;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        err("socket");
    }

    int opt = SO_REUSEADDR;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        err("setsockopt");
    }

    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;                // host byte order
    server.sin_port = htons(PORT);              // short, network byte order
    server.sin_addr.s_addr = htonl(INADDR_ANY); // automatically fill with my IP

    if (bind(listenfd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1) {
        err("bind");
    }else{
        cout << "Socket succefully created and binded\n";
    }

    if (listen(listenfd, BACKLOG) == -1) {
        err("listen");
    }else{
        cout << "Server listening ...\n";
    }

    sa.sa_handler = sigchld_handler;  // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        err("sigaction");
    }


    while (serverON) {     // main accept() loop
        sin_size = sizeof(struct sockaddr_in);

        connectfd = accept(listenfd, (struct sockaddr *)&client, &sin_size);
        cout << "Server detected new connection\n";

        // // Send msg to clients
        // string data;
        // demo::People to;
        // to.set_name("Lysting Huang");
        // to.set_id(123);
        // to.set_email("lytsing@hotmail.com");
        // to.SerializeToString(&data);
        // char bts[data.length()];
        // sprintf(bts, "%s", data.c_str());
        // send(connectfd, bts, sizeof(bts), 0);
        pthread_t conection_thread;
        pthread_create(&conection_thread, NULL, conHandler, (void *)connectfd);
    }

    close(listenfd);

    return 0;
}
