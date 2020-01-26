#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 


typedef struct Node{
	void *data;
	struct Node *next;
}Node;

typedef struct Queue{
	Node *first;
	Node *last;
}Queue;

typedef struct Caja{
	int capacidad;
  	int recibidos;
  	int estado; // 1 -> En procesamiento, 2 -> Empaquetada
}Caja;

typedef struct Brazo{
	int estado;	// 0 -> Detenido, 1 -> Iniciado
  	Queue *Cajas; // Cajas asignadas
  	Queue *items; // Items asignados
}Brazo;

// Rutinas de hilos
void *routine_receptor();		// Se comunica con el generador de paquetes
void *routine_configuracion();  // Se comunica con la consola de ingreso de parametros
void *routine_asignador();      // Asigna a los brazos los paquetes/items recibidos
void *routine_estado();			// Imprime el estado de brazos y paquetes
// Funciones
Queue *newQueue();
Brazo *newBrazo();
Caja *newCaja(int capacidad);
void add(void *element, Queue *cola);
int size(Queue *cola);
void *pop(Queue *cola);


int main(){
	return 1;
}

void *routine_receptor(){
	//Declaring process variables.
	int server_sockfd, client_sockfd;
	int server_len ; 
	int rc ; 
	unsigned client_len;
	struct sockaddr_in server_address;
	struct sockaddr_in client_address;
	char buffer[50]; 

	//Remove any old socket and create an unnamed socket for the server.
	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htons(INADDR_ANY);
	server_address.sin_port = htons(7734) ; 
	server_len = sizeof(server_address);

	rc = bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
	printf("RC from bind = %d\n", rc ) ; 
	
	//Create a connection queue and wait for clients
	rc = listen(server_sockfd, 5);
	printf("RC from listen = %d\n", rc ) ; 

	client_len = sizeof(client_address);
	client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_address, &client_len);
	printf("after accept()... client_sockfd = %d\n", client_sockfd) ; 

	while(1)
	{
		memset(buffer,0,sizeof(buffer));
		rc = read(client_sockfd, &buffer,sizeof(buffer));
         	if (rc == -1 || rc == 0) break ; 
		printf("[Data = %s rc=%d]\n",buffer,rc);	
	}

	printf("server exiting\n");

	close(client_sockfd);
	return NULL
}

void *routine_configuracion(){
	key_t key; 
	int msgid;
	float k_received = 0;
	float diff = 0;
	int i = 0;

	// ftok to generate unique key 
	key = ftok("token", 65); 

	// Crear message queue
	msgid = msgget(key, 0666 | IPC_CREAT); 

	// Recibo primer mensaje, indicará el tiempo T
	msgrcv(msgid, &message, sizeof(message), 1, 0); 
	int time = atoi(message.mesg_text);
	// si se recibe un valor válido se actualiza el T default
	if (time > 0){
		T = time;
		sem_wait(&timex);
		TIMER = T*3;
		sem_post(&timex);
	}

	while (!END){
		// msgrcv to receive message 
		msgrcv(msgid, &message, sizeof(message), 1, 0); 

		// Validación AZ5
		if (strcmp(message.mesg_text, "az5\n") == 0){			
			// Liberar barras
			for (i=0; i<N; i++){
				pistones[i]->free();
			}
			strcpy(alert_msg, "Las barras han sido liberadas");
			END = true;
			break;
		}

		// convert to float
		k_received = atof(message.mesg_text);	

		// Esperar para actualizar K
		sem_wait(&mutex);		

		// Actualizar K
		K = K + k_received;	
		diff = 1 - K;		
		
		// Liberar K
		sem_post(&mutex);
	}

	// to destroy the message queue 
	msgctl(msgid, IPC_RMID, NULL); 
	
	return NULL;
}


Queue *newQueue(){
  	Queue *cola = NULL;
  	cola = (Queue*)malloc(sizeof(Queue));  	
	cola->first = NULL;
  	cola->last = NULL;
  	return cola;
}

Brazo *newBrazo(){
	Brazo *b = NULL;
  	b = (Brazo*)malloc(sizeof(Brazo));
  	b->Cajas = newQueue();
  	b->items = newQueue();
  	b->estado = 1;
  	return b;
}

Caja *newCaja(int capacidad){
	Caja *c = NULL;
  	c = (Caja*)malloc(sizeof(Caja));
  	c->capacidad = capacidad;
  	c->recibidos = 0;
  	c->estado = 1;
  	return c;
}

void add(void *element, Queue *cola){
	Node *new = NULL;
  	new = (Node*)malloc(sizeof(Node));
  	new->next = (Node*)malloc(sizeof(Node));
	new->data = element;
  	new->next = NULL;  	
  	if (cola->first == NULL){
      cola->first = new;
      cola->last = new;
    } else{        
      cola->last->next = new;
	  cola->last = new;            
    }
}

int size(Queue *cola){
  	int cont = 0;
  	Node *n = cola->first;  	
    while(n != NULL){
      cont++;      
      n = n->next;
    }    
  	return cont;
}

void *pop(Queue *cola){
  	int tam = size(cola);
  	if (tam > 0){
    	Node *first = cola->first;
		void *data = first->data;
		cola->first = first->next;
      	if (tam == 1){
          cola->last = NULL;
        }
      	free(first);
		return data;
    }else{
      	return NULL;
    }
}