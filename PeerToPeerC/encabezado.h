#ifndef HEADER_H_
#define HEADER_H_

#include <stdint.h>

/*Códigos de operación*/
#define IP 	          1 	//Solicita la ip
#define ELIMINACION 	2  	//Elimina los archivos de la lista (Se castea a string)
#define CREACION 	    3 	//Crea los archivos de la lista (Se castea a string)
#define SOLICITUD 	  4  	//Desea obtener paquetes de archivos
#define ENVIO         5   //Envía los paquetes de archivos


/*Códigos de error */
#define OK 		0	//Operación correcta
#define E_BAD_OPCODE 	-1 	//Operación desconocida
#define E_BAD_PARAM 	-2 	//Error en un parámetro
#define E_IO 		-3 	//Error en disco u otro error de E/S


//#define MAX_LIST 1000 	//Longitud máxima de la lista de archivos
#define BUF_SIZE 3000 	//Cantidad de bytes que se transfieren por paquete

struct mensajeU//Mensaje Universal
{
  //Código de operación
  uint32_t op;
  // Numero de bytes útiles
  uint32_t count;
  // Código de error
  uint32_t result;
  //Listado de nombres de archivos
  //char lista[BUF_SIZE];
  // Lugar del archivo donde se debe iniciar la lectura o escritura
  uint32_t offset;
  // Datos del archivo
  char data[BUF_SIZE];
};

#endif
