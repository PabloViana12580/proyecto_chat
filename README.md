# proyecto_chat

### servidor y cliente de un chat en c++ utilizando sockets y pthreads
#### Instrucciones para compilar el protocolo: `protoc -I=. --cpp_out=. mensaje.proto`

#### Instrucciones para compilar cliente: `g++ client.cpp mensaje.pb.cc -lprotobuf -o cliente -pthread -std=c++11`
#### Instrucciones para correr el cliente: `./cliente <username> <ip_server> <puerto>`

#### Instrucciones para compilar el server-test: `g++ server-test.cpp mensaje.pb.cc -lprotobuf -o server -pthread -std=c++11`
#### Instrucciones para correr el server: `./server <puerto>`
