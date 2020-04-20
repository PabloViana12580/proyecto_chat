# proyecto_chat

### servidor y cliente de un chat en c++ utilizando sockets y pthreads
#### Instrucciones para compilar el protocolo: `protoc -I=. --cpp_out=. mensaje.proto`

#### Instrucciones para compilar cliente: `g++ client.cpp mensaje.pb.cc -lprotobuf -o cliente -std=c++11`

#### Instrucciones para compilar el server-test: `g++ server-test.cpp mensaje.pb.cc -lprotobuf -o server -std=c++11`
