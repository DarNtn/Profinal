#include <stdio.h> 
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
*/


// structure for message queue 
struct struct_msg { 
	long mesg_type; 
	char mesg_text[10]; 
} message; 

int isNumber(char *cad);
int validarComando(char *str, int robots);

int main() 
{ 
	key_t key; 
	int msgid;
	int robots = 0;

	// ftok to generate unique key 
	key = ftok("token", 65); 
	
	msgid = msgget(key, 0666 | IPC_CREAT); 
	message.mesg_type = 1; 

	system("clear");
	printf("\t\t\tCar-Assembly Robotic System\n\n");

	printf("Cantidad de brazos robots: ");
	fgets(message.mesg_text, 10, stdin);
	while(!isNumber(message.mesg_text)){
		printf("Ingrese un valor entero: ");
		fgets(message.mesg_text, 10, stdin);
		printf("ingreso: %s\n",message.mesg_text);
	}
	robots = atoi(message.mesg_text);
	msgsnd(msgid, &message, sizeof(message), 0);

	system("clear");
	printf("\tComandos:\n\t\tParar robot: stop [n]\n\t\tReanudar robot: start [n]\n\n");
	while(1){
		printf(">> "); 
		fgets(message.mesg_text, 10, stdin);

		if (validarComando(message.mesg_text, robots)){
			// msgsnd to send message 
			msgsnd(msgid, &message, sizeof(message), 0);
		}
	}	

	return 0; 
} 

int isNumber(char *str){
    char *i=str;
    while (*i!= '\0' && *i!= '\n') 
    {       	
        if (isdigit(*i) == 0) 
            return 0;
        i++;
    }
    return 1;
}

int validarComando(char *str, int robots){  	
	char *token = strtok(str, " "); 
			
  	if (strcmp(token, "start")!=0 && strcmp(token, "stop")!=0){
      	printf("Comando incorrecto\n");
      	return 0;
    }		
  	token = strtok(NULL, " "); 
	if (!isNumber(token) || atoi(token) < 1 || atoi(token) > robots){
      	printf("Comando incorrecto. Robots válidos entre [1-%d]\n",robots);
      	return 0;
    }
    token = strtok(NULL, " ");
  	if (token != NULL){
		printf("Comando incorrecto\n");
      	return 0;
    }
  	return 1;
}