peerPrototipo: SocketDatagrama.o PaqueteDatagrama.o
	g++ peerPrototipo.cpp -std=c++0x -pthread  SocketDatagrama.o PaqueteDatagrama.o -o peerPrototipo
SocketDatagrama.o: SocketDatagrama.cpp SocketDatagrama.h
	g++ SocketDatagrama.cpp -c
PaqueteDatagrama.o: PaqueteDatagrama.cpp PaqueteDatagrama.h
	g++ PaqueteDatagrama.cpp -c
