all: server client
server: server.cc codec.h
	g++ -std=c++11 -o server server.cc -pthread -lmuduo_base -lmuduo_net
client: client.cc codec.h
	g++ -std=c++11 -o client client.cc -pthread -lmuduo_base -lmuduo_net
