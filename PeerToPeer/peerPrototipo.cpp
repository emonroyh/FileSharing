#include <inttypes.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <fcntl.h>
#include <unistd.h>

#include <vector> //Vector

#include <time.h>

#include  <thread>

#include <dirent.h>//Buesqueda de directorios
#include <errno.h>

#include <sstream>//split
#include <iostream>

#include <fstream>      // convertInt

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "SocketDatagrama.h"
#include "PaqueteDatagrama.h"
//#include "header.h"
#include "encabezado.h"

//g++ peerPrototipo.cpp -std=c++0x -pthread SocketDatagrama.o PaqueteDatagrama.o -o peerPrototipo
//.peerPrototipo

#define MAX_PATH 255 //Longitud máxima en el nombre de un archivo

/* Definición de las operaciones permitidas */
#define CREATE 1 //Crea un nuevo archivo
#define READ 2 //Lee una parte del archivo
#define WRITE 3 //Escribe una parte del archivo
#define DELETE 4 //Elimina un archivo existente

struct messageCS{
 uint32_t opcode; //Código de la operación solicitada por el cliente
 uint32_t count; //Numero de bytes útiles (leídos o por escribir), devueltopor el servidor en READ y enviado por el cliente en WRITE
 uint32_t offset; //Lugar del archivo donde se debe iniciar la lectura o escritura, enviado por el cliente
 char name[MAX_PATH]; //Nombre del archivo sobre el cual se opera
};
/* Formato del mensaje que viaja del servidor hacia el cliente */
struct messageSC{
 uint32_t count; //Numero de bytes útiles, devuelto por el servidor en READ y enviado por el cliente en WRITE
 uint32_t result; //Código de error devuelto por el servidor
 char data[BUF_SIZE]; //Datos del archivo
};

#define ptoCArch 8011
#define ptoSArch 8012
#define ptoEcho  8013

#define ptoCArchC 8011
#define ptoSArchC 8012
#define ptoEchoC  8013


/*
#define ptoCArchC 8001
#define ptoSArchC 8002
#define ptoEchoC  8003

#define ptoCArch 8011
#define ptoSArch 8012
#define ptoEcho  8013
*/


#define TIEMPO    5 //Tiempo de espera para obtener las direcciones IP
char SEPARADOR = '@';

void waitFor (unsigned int secs);
int retUltimo(string direccion);
string retBase(string direccion);
int getdir (string dir, vector<string> &files);

string obtieneLista(string dir);
void elimina(string K, string listaTrash);
vector<string> separaLista(string lista);
string convertInt(int number);
string verficar(string K, string listaNueva);

int leer(mensajeU *men, mensajeU *datos);

void buclePrincipal();
void clienteArchivos();
void servidorArchivos();
void servidorEcho();

string dirTrash = "";
string dirKazaa = "";

string listaTrashMultiL = "";
string listaTrashMultiF = "";

int main(int argc, char const *argv[])
{
  dirKazaa = argv[1];
  dirTrash = argv[2];

  thread hilo1(buclePrincipal);
  thread hilo2(clienteArchivos);
  thread hilo3(servidorArchivos);
  thread hilo4(servidorEcho);
  hilo1.join();
  hilo2.join();
  hilo3.join();
  hilo4.join();

  while(true){

  }

  return 0;
}

void buclePrincipal()
{
  string base;
  int dir[255];
  int cont = 0;

  mensajeU solicitudDir;
  solicitudDir.op = IP;

  SocketDatagrama socketDir = SocketDatagrama();
  time_t endwait;
  time_t start;


  while(true){
    int ans;
    ans = socketDir.activaBroadcast();
    if(ans < 0){
      printf("buclePrincipal:\t Hubo un error al poner el socket en broadcast\n");
      return;
    }

    PaqueteDatagrama out = PaqueteDatagrama((char*)&solicitudDir, sizeof(solicitudDir), "", ptoEchoC);

    socketDir.setTiempoEspera(3, 0);

    start = time(NULL);
    endwait = start + TIEMPO;

    cont = 0;
    ans = socketDir.envia(out);

    while(start < endwait){//Obtuvo todas las direcciones IP (Proceso 0)
      PaqueteDatagrama in = PaqueteDatagrama(sizeof(mensajeU));
      ans = socketDir.recibe(in);
      if(ans > 0){
        dir[cont] = retUltimo(string(in.obtieneDireccion()));
        base = retBase(string(in.obtieneDireccion()));
        cont++;
      }
      start = time(NULL);
    }
    cont--;

    //(Proceso 1)
    string listaTrash = "";
    listaTrash = obtieneLista(dirTrash);
    printf("buclePrincipal:\t Lista Basura local: %s\n", listaTrash.c_str());
    elimina(dirKazaa, listaTrash);


    //(Proceso 2)
    ans = socketDir.activaBroadcast(0);//Desactiva el broadcast
    if(ans < 0){
      printf("buclePrincipal:\t Hubo un error al poner el socket en broadcast\n");
      return;
    }

    //Proceso 3
    string listaKazaa = obtieneLista(dirKazaa);
    mensajeU enviaListaArch;
    //bzero((char *)&enviaListaArch, sizeof(mensajeU));
    memset((char *)&enviaListaArch, '\0', sizeof(mensajeU));
    mensajeU enviaListaTrash;
    //bzero((char *)&enviaListaTrash, sizeof(mensajeU));
    memset((char *)&enviaListaTrash, '\0', sizeof(mensajeU));


    enviaListaTrash.op = ELIMINACION;
    enviaListaArch.op = CREACION;
    strncpy(enviaListaTrash.data, listaTrash.c_str(), listaTrash.size());
    strncpy(enviaListaArch.data, listaKazaa.c_str(),listaKazaa.size());
    enviaListaTrash.count = listaTrash.size();
    enviaListaArch.count = listaKazaa.size();

    int i;
    for(i = 0; i <= cont; i++){
      string auxDir = base + convertInt(dir[i]);
      char *auxD = &auxDir[0u];
      PaqueteDatagrama outT = PaqueteDatagrama((char*)&enviaListaTrash, sizeof(enviaListaTrash), auxD, ptoEchoC);
      socketDir.envia(outT);
      waitFor(3);
      PaqueteDatagrama outK = PaqueteDatagrama((char*)&enviaListaArch, sizeof(enviaListaArch), auxD, ptoCArchC);
      socketDir.envia(outK);
      cout <<"buclePrincipal:\t Mensajes de ELIMINACION y CREACION envados a: " << auxDir << endl;
      printf("buclePrincipal:\tLista Trash %s\n", listaTrash.c_str());
      printf("buclePrincipal:\tLista Arch %s\n", listaKazaa.c_str());
    }

  }

}





void clienteArchivos()
{
  int countSer;
  int banderaSalida = 0;
  int i;
  SocketDatagrama socketCArch(ptoCArch);
  char lista[BUF_SIZE];
  string nuevosArch;
  string aux;
  const char * caux;
  vector<string> vArch;
  while (true) {
    //bzero((char *)&lista, BUF_SIZE*sizeof(char));
    memset((char *)&lista, '\0', BUF_SIZE*sizeof(char));
    banderaSalida = 0;
    PaqueteDatagrama in = PaqueteDatagrama(sizeof(mensajeU));
    socketCArch.recibe(in);
    mensajeU * dataIn;
    dataIn = (mensajeU*)in.obtieneDatos();
    if(dataIn->op == CREACION){
      strncpy ( lista, dataIn->data, dataIn->count);
      printf("clienteArchivos: La lista foranea es: %s\n", lista);
      nuevosArch = verficar(dirKazaa, lista);
      printf("clienteArchivos: Los archivos que no tengo son: %s\n", nuevosArch.c_str());
      vArch.clear();
      vArch = separaLista(nuevosArch);

        for(i = 0; i < vArch.size(); i++){
          countSer = 0;
          int destino;
          SocketDatagrama socket = SocketDatagrama();
          messageCS mensaje;
        	strcpy(mensaje.name, vArch[i].c_str());
          PaqueteDatagrama in2 = PaqueteDatagrama(sizeof(messageSC));
          socket.setTiempoEspera(2, 0);
          int offset = 0, ans = 0;
        	messageSC * dataIn2;

          string temp = dirKazaa + vArch[i];
          destino = open(temp.c_str(), O_WRONLY|O_TRUNC|O_CREAT, 0666);
          int peticion = 1, perdidas = 0;

          do {
        		mensaje.opcode = READ;
        		mensaje.count = 0;
        		mensaje.offset = offset;
        		PaqueteDatagrama out = PaqueteDatagrama((char*)&mensaje, sizeof(mensaje),in.obtieneDireccion(), ptoSArchC);

        		ans = 0;
        		socket.envia(out);
        		ans = socket.recibe(in2);
        		while(ans == -1) {
              printf("***************************Servidor tarda en responder\n" );
        			socket.envia(out);
        			ans = socket.recibe(in2);
        			perdidas++;
              countSer++;
              if(countSer >= 5){
                printf("***************************Parece que el servidor está muerto\n");
                close(destino);
                printf("Abortando archivo\n");
                aux = dirKazaa + mensaje.name;
                caux = aux.c_str();
                if( remove( caux ) != 0 )
                  perror( "Error borrando el archivo" );
                else
                  puts( "Archivo borrado" );

                banderaSalida = 1;
                break;
              }
        		}
            countSer = 0;
            if(banderaSalida == 1)break;
        		dataIn2 = (messageSC *) in2.obtieneDatos();
        		if(OK == dataIn2->result)
        		{
        			write(destino, dataIn2->data, dataIn2->count);
        		}
        		else
        		{
        			printf("Recibe error de %s:%d, error %d\n",in.obtieneDireccion(), in.obtienePuerto(),dataIn->result);
              close(destino);
              printf("Ese archivo no existia, error de transmisión\n");
              aux = dirKazaa + mensaje.name;
              caux = aux.c_str();
              if( remove( caux ) != 0 )
                perror( "Error borrando el archivo" );
              else
                puts( "Archivo borrado" );

              banderaSalida = 1;
              break;
        		}
        		offset += dataIn2->count;
        	} while((BUF_SIZE - dataIn2->count) == 0);
          if(banderaSalida == 1)break;
          close(destino);
        	printf("Archivo recibido bytes: %d\n\n", offset);
          if(offset == -1 || offset == 0){
            printf("Ese archivo no existia, error de transmisión\n");
            aux = dirKazaa + mensaje.name;
            caux = aux.c_str();
            if( remove( caux ) != 0 )
              perror( "Error borrando el archivo" );
            else
              puts( "Archivo borrado" );

          }
        }
    }
  }
  return;
}




void servidorArchivos()//Digamos que ya está bien
{
  struct messageSC mensEnv;
  struct messageCS mensRcb;
  int pid, estado, fileDescriptor;

  //bzero((char *)&mensEnv, sizeof(messageSC));
  //bzero((char *)&mensRcb, sizeof(messageCS));

  memset((char *)&mensEnv, '\0', sizeof(messageSC));
  memset((char *)&mensRcb, '\0', sizeof(messageCS));

  PaqueteDatagrama envio((char *)(&mensEnv), sizeof(messageSC), "", 0);
  //PaqueteDatagrama recepcion(sizeof(struct messageCS));
  SocketDatagrama socketlocal(ptoSArch);
  PaqueteDatagrama recepcion(sizeof(messageCS));
  //Crea Demonio
  cout << "Este es un Demonio servidor de archivos \n";
  while(1){
     pid = fork();
     switch(pid){
        case -1:
           perror("Error en el fork");
           socketlocal.~SocketDatagrama();
           exit(-1);
           break;
        case 0: //Hijo
           cout << "Crea un proceso hijo para atender a clientes\n";

           while(1){
              //recvfrom(s, (char *) &mensaje, sizeof(struct message), 0, (struct sockaddr *)&msg_to_client_addr, &clilen);
              cout << "Esperando solicitud ...\n";
              socketlocal.recibe(recepcion);
              cout << "\nLa IP de quien me envía es: " << recepcion.obtieneDireccion() << endl;
              cout << "El puerto de quien me envía es: " << recepcion.obtienePuerto() << endl;
              //recepcion.~PaqueteDatagrama();
              memcpy((char *)&mensRcb, recepcion.obtieneDatos(), sizeof(mensRcb));
              cout << "Opción recibida: = " << mensRcb.opcode << endl;
              cout << "count: = " << mensRcb.count << endl;
              cout << "offset: = " << mensRcb.offset << endl;
              cout << "nombre: " << mensRcb.name << endl;
              cout << "Mi estructura messageCS " << sizeof(struct messageCS);// << " mensEnv= " sizeof(struct messageCS) << " Bytes" << endl;
              if(mensRcb.opcode == READ){
                    //printf("Solicitud para leer del archivo: %s desde un offset = %ld\n", mensaje.name, mensaje.offset);
                 string aux = dirKazaa + mensRcb.name;
                 fileDescriptor = open(aux.c_str(), O_RDONLY);
                 if(fileDescriptor == -1)
                    mensEnv.result = E_IO;
                 else{
                    lseek(fileDescriptor, mensRcb.offset, SEEK_SET);
                    mensEnv.count = read(fileDescriptor, mensEnv.data, BUF_SIZE);
                    mensEnv.result = OK;
                    close(fileDescriptor);
                 }
                 envio.inicializaDatos((char *) &mensEnv);
                 envio.inicializaIp(recepcion.obtieneDireccion());
                 envio.inicializaPuerto(recepcion.obtienePuerto());
                 //socketlocal.envia(envio);
                 socketlocal.envia(envio);
              }
           }
           exit(0);
           break;
        default: //padre
           wait(&estado);
           cout << "Soy el padre y acaba de morir un proceso hijo. Se crea otro...\n";

     }
  }
  return;
}



void servidorEcho()//Proceso 5
{
  SocketDatagrama socketEcho(ptoEcho);
  char lista[BUF_SIZE];

  while (true) {
      //bzero((char *)&lista, BUF_SIZE*sizeof(char));
      memset((char *)&lista, '\0', BUF_SIZE*sizeof(char));
      PaqueteDatagrama in = PaqueteDatagrama(sizeof(mensajeU));
      socketEcho.recibe(in);
      mensajeU * dataIn;
      dataIn = (mensajeU*)in.obtieneDatos();
      if(dataIn->op == IP){
        socketEcho.envia(in);
      }else if(dataIn->op == ELIMINACION){
        strncpy ( lista, dataIn->data, dataIn->count);
        elimina(dirKazaa, lista);
      }else{
        printf("servidorEcho:\t Se recibió una solicitud que no es IP o ELIMINACION\n");
        printf("servidorEcho:\t %d", dataIn->op);
      }

  }
  return;
}





string convertInt(int number)
{
   stringstream ss;
   ss << number;
   return ss.str();
}


string verficar(string K, string listaNueva)
{
  int i, j, bandera = 0;
  string listaArchivos = "";
  string listaKazaa = obtieneLista(K);
  printf("verificar:\t Lista Kazaa local: %s\n", listaKazaa.c_str());
  printf("verificar:\t Lista Kazaa foránea: %s\n", listaNueva.c_str());

  vector<string> vNueva;
  vNueva.clear();
  vector<string> vKazaa;
  vKazaa.clear();


  vKazaa = separaLista(listaKazaa);
  vNueva = separaLista(listaNueva);

  for(i = 0; i < vNueva.size(); i++){
    for(j = 0; j < vKazaa.size(); j++){
      if(vKazaa[j] == vNueva[i]){
        bandera = 1;
      }
    }
    if(bandera == 0){
      printf("verificar:\t No tengo %s\n", vNueva[i].c_str());
      if(listaArchivos.empty()){
        listaArchivos = vNueva[i];
      }else{
        listaArchivos = listaArchivos + SEPARADOR + vNueva[i];
      }
    }
    bandera = 0;
  }

  return listaArchivos;
}

void elimina(string K, string listaTrash)
{
  int i, j;
  string listaKazaa = obtieneLista(K);
  printf("elimina:\t Lista Kazaa local: %s\n", listaKazaa.c_str());
  printf("elimina:\t Lista Trash recibida: %s\n", listaTrash.c_str());


  vector<string> vTrash;
  vTrash.clear();
  vector<string> vKazaa;
  vKazaa.clear();

  vKazaa = separaLista(listaKazaa);
  vTrash = separaLista(listaTrash);

  for(i = 0; i < vKazaa.size(); i++){
    for(j = 0; j < vTrash.size(); j++){
      if(vKazaa[i] == vTrash[j]){
        printf("elimina:\t %s de Kazaa local aparece en listaTrash recibida\n", vKazaa[i].c_str());
        string aux = K + vKazaa[i];
        const char * caux = aux.c_str();
        if( remove( caux ) != 0 )
          perror( "Error borrando el archivo" );
        else
          puts( "Archivo borrado" );
      }
    }
  }

}

vector<string> separaLista(string lista){
  vector<string> v;
  v.clear();
      istringstream g(lista);
      string e;
      while (getline(g, e, SEPARADOR)) {
          v.push_back(e);
      }

  return v;
}


string obtieneLista(string dir)
{
  string list = string(dir);//Obtiene una lista los archivos en dir
  vector<string> files = vector<string>();
  files.clear();
  getdir(list,files);
  string firstLetter(".");
  string firstLetter1("..");
  string firstLetter2(" ");
  string firstLetter3("");
  string lista = "";
  for (unsigned int i = 0; i < files.size();i++) {

    if(lista == "")
      if(files[i] == firstLetter || files[i] == firstLetter1 || files[i] == firstLetter2 || files[i] == firstLetter3){
      }else{
        lista = files[i];
      }
    else
      if(files[i] == firstLetter || files[i] == firstLetter1 || files[i] == firstLetter2 || files[i] == firstLetter3){
      }else{
        lista = lista + SEPARADOR + files[i];
      }

  }
  cout << "Lista: " << lista << endl;
  return lista;
}



int getdir (string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL) {
        cout << "detdir: Error(" << errno << ") abriendo " << dir << endl;
        return errno;
    }

    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(string(dirp->d_name));
    }
    closedir(dp);
    return 0;
}




int retUltimo(string direccion){
  int first, len = direccion.length();

  int pos = direccion.find(".");
  if (pos != string::npos) {
    while(true){
      pos = direccion.find('.', pos + 1);
      if (pos != string::npos) {
        first = pos;
      }else{
        break;
      }
    }
  }

  string num = direccion.substr(first+1, len);
  return atoi(num.c_str());
}

string retBase(string direccion){

  int first, len = direccion.length();

  int pos = direccion.find(".");

  if (pos != string::npos) {
    while(true){
      pos = direccion.find('.', pos + 1);
      if (pos != string::npos) {
        first = pos;
      }else{
        break;
      }
    }
  }

  string base = direccion.substr(0, first+1);
  return base;
}

void waitFor (unsigned int secs) {
    unsigned int retTime = time(0) + secs;   // Get finishing time.
    while (time(0) < retTime);               // Loop until it arrives.
}
