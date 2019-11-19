#include "SocketDatagrama.h"
#include <errno.h>

SocketDatagrama::SocketDatagrama(int puerto)
{
    s = socket(AF_INET, SOCK_DGRAM, 0);
    //bzero((char *)&direccionLocal, sizeof(direccionLocal));
    memset((char *)&direccionLocal, '\0',sizeof(direccionLocal));
    direccionLocal.sin_family = AF_INET;
    direccionLocal.sin_addr.s_addr = INADDR_ANY;
    direccionLocal.sin_port = htons(puerto);
    bind(s, (struct sockaddr *)&direccionLocal,sizeof(direccionLocal));
}

SocketDatagrama::~SocketDatagrama()
{
//    close(s);
}


int SocketDatagrama::recibe(PaqueteDatagrama & p)
{
    unsigned int addr_len = sizeof(direccionForanea);
    //bzero((char *)&direccionForanea, sizeof(direccionForanea));
    memset((char *)&direccionForanea, '\0',sizeof(direccionForanea));
    int regreso = recvfrom(s,p.obtieneDatos(),p.obtieneLongitud(), 0, (struct sockaddr *) &direccionForanea, &addr_len);

    p.inicializaPuerto(ntohs(direccionForanea.sin_port));
    p.inicializaIp(inet_ntoa(direccionForanea.sin_addr));
    //std::cout << "Mensaje recibido de: " << inet_ntoa(direccionForanea.sin_addr) << ":" << ntohs(direccionForanea.sin_port) << std::endl;
    return regreso;
}

int SocketDatagrama::recibeTimeout(PaqueteDatagrama & p)
{
  socklen_t clilen = sizeof(direccionForanea);
  //bzero((char *)&direccionForanea, sizeof(direccionForanea));
  memset((char *)&direccionForanea, '\0', sizeof(direccionForanea));
  int n;
  n = recvfrom(s, (char *)p.obtieneDatos(), p.obtieneLongitud(),
              0, (struct sockaddr *)&direccionForanea, &clilen);

  return n;
}



int SocketDatagrama::envia(PaqueteDatagrama & p)
{
    //bzero((char *)&direccionForanea, sizeof(direccionForanea));
    memset((char *)&direccionForanea, '\0', sizeof(direccionForanea));
    direccionForanea.sin_family = AF_INET;
    direccionForanea.sin_addr.s_addr = inet_addr(p.obtieneDireccion());
    direccionForanea.sin_port = htons(p.obtienePuerto());

    return sendto(s,p.obtieneDatos(), p.obtieneLongitud(), 0, (struct sockaddr *) &direccionForanea, sizeof(direccionForanea));
}

int SocketDatagrama::activaBroadcast(bool opcion)
{
	int yes = opcion ? 1 : 0;
	return setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes,sizeof(int));
}


void SocketDatagrama::setTiempoEspera(time_t segundos, suseconds_t microsegundos)
{
    tiempofuera.tv_sec = segundos;
    tiempofuera.tv_usec = microsegundos;
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char *)&tiempofuera, sizeof(tiempofuera));
    return;
}

int SocketDatagrama::setBroadcast(){
  int yes=1, n;
  n = setsockopt(s, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int));
  return n;
}
