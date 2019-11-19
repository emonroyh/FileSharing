#ifndef HEADER_H_
#define HEADER_H_

#include <stdint.h>

/* Definiciones necesarias para los clientes y el servidor de archivos */

#define BUF_SIZE 3000 	//Cantidad de bytes que se transfieren por paquete

/*Códigos de operación*/
#define IP 	          1 	//Solicita la ip
#define ELIMINACION 	2  	//Elimina los archivos de la lista (Se castea a string)
#define CREACION 	    3 	//Crea los archivos de la lista (Se castea a string)
#define SOLICITUD 	  4  	//Desea obtener paquetes de archivos
#define ENVIO         5   //Envía los paquetes de archivos

/*Códigos de error */
#define OK 0 //Operación correcta
#define E_BAD_OPCODE -1 //Operación desconocida
#define E_BAD_PARAM -2 //Error en un parámetro
#define E_IO -3 //Error en disco u otro error de E/S

/* Formato del mensaje que viaja del cliente hacia el servidor. El tipo de dato numérico
uint32_t está definido en inttypes.h y es consistente entre computadoras distintas e
independiente del sistema operativo */

struct mensajeU//Mensaje Universal
{
  uint32_t op;  //Código de operación
  uint32_t count;  // Numero de bytes útiles
  uint32_t result;  // Código de error
  uint32_t offset;  // Lugar del archivo donde se debe iniciar la lectura o escritura
  char data[BUF_SIZE];  // Datos del archivo
};

#endif
