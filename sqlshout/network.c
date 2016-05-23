#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <shout/shout.h>

#include "types.h"
#include "utils.h"

/* Begin function prototypes */

int hostlookup(char *hostname, char *address) ;

char *strip_white_space(char *input) ;

char *spawn_listener(program_t *prog) ;

/* End function prototypes */

/* Begin constant definitions */

#define BINDPORT 8010
#define BACKLOG 5

/* End Constant definitions */

extern FILE *debugfp ;

void *send_cancel (program_t *prog)
{
	int errVal ;
	char *username, *password ;

	username = malloc(15) ;
	if (username==NULL) {
		fprintf(debugfp,"[%s] Unable to allocate memory for username.\n",logTime()) ;
		return NULL ;
	} else {
		username = memcpy(username,"\0",14) ;
	}

	password = malloc(16) ;
	if (password==NULL) {
		fprintf(debugfp,"[%s] Unable to allocate memory for username.\n",logTime()) ;
		return NULL ;
	} else {
		password = memcpy(password,"\0",15) ;
	}

	if (prog->connection==-1) {
		fprintf(debugfp,"[%s] Failed to successfully accept connection.\n",logTime()) ;
		perror("ACCEPT") ;
		return NULL ;
	}
		
	errVal = send(prog->connection, "Enter username: ", 15, 0) ;
	if (errVal==-1) {
		fprintf(debugfp,"[%s] Error sending connect string.\n",logTime()) ;
		perror("SEND") ;
		return NULL ;
	}

	errVal = recv(prog->connection,username,14,0) ;
	if (errVal==-1) {
		fprintf(debugfp,"[%s] Error reading from socket.\n",logTime()) ;
		perror("RECV") ;
		return NULL ;
	}
	if (username==NULL) {
		fprintf(debugfp,"[%s] Unable to parse username.\n",logTime()) ;
		return NULL ;
	} else {
		fprintf(debugfp,"[%s] Accepted username for %s.\n",logTime(),strip_white_space(username)) ;
	}

	errVal = send(prog->connection, "Enter password: ", 15, 0) ;
	if (errVal==-1) {
		fprintf(debugfp,"[%s] Error sending connect string.\n",logTime()) ;
		perror("SEND2") ;
		return NULL ;
	}

	errVal = recv(prog->connection,password,14,0) ;
	if (errVal==-1) {
		fprintf(debugfp,"[%s] Error reading from socket.\n",logTime()) ;
		perror("RECV2") ;
		return NULL ;
	}
	if (password==NULL) {
		fprintf(debugfp,"[%s] Unable to parse password.\n",logTime()) ;
		return NULL ;
	} else {
		fprintf(debugfp,"[%s] Accepted password %s.\n",logTime(),strip_white_space(password)) ;
	}
		
	if ((strcmp(strip_white_space(username),"DSpair")==0) && (strcmp(strip_white_space(password),"Bal33nwhal3")==0)) {
		fprintf(debugfp,"[%s] Sending cancel request to reencoder thread %i.\n",logTime(),*(int *)prog->encoder) ;
		errVal = pthread_cancel(*(int *)prog->encoder) ;
		if (errVal!=0) {
			perror("CANCEL") ;
		}
	}
	fflush(debugfp) ;
	
	errVal = close(prog->connection) ;
	if (errVal==-1) {
		perror("CLOSE") ;
		return NULL ;
	}
	
	return NULL ;
}

char *strip_white_space(char *input)
{
	int outputpos=0, looper, length ;
	char *output ;
	
	length = strlen(input) ;
	output = malloc(length+1) ;
	output = memcpy(output,"\0",length+1) ;
	for (looper=0;looper<length;looper++) {
		if ((input[looper]!='\n') && (input[looper]!=' ') && (input[looper]!='\t') && (input[looper]!='\v') && (input[looper]!='\r')) {
			output[outputpos] = input[looper] ;
			outputpos=outputpos + 1 ;
		} else {
			break;
		}
	}
	
	output[outputpos] = '\0' ;
	return output ;
}	

int hostlookup (char *hostname, char *address)
{
	struct hostent *h;

	if ((h=gethostbyname(hostname)) == NULL) {  
		herror("gethostbyname");
		return 1 ;
	}

	address = inet_ntoa(*((struct in_addr *)h->h_addr));
	       
	return 0;
}

char *spawn_listener(program_t *prog) 
{
	struct protoent *protocol ;
	struct sockaddr_in socketinfo ; 
	struct sockaddr clientinfo ;
	char *username, *password ;
	int listensocket, errVal ;
	unsigned int addrSize ;

	fprintf(debugfp,"[%s] Spawned network socket listener.\n",logTime()) ;
	
	protocol = getprotobyname("TCP");
	listensocket = socket (PF_INET,SOCK_STREAM,protocol->p_proto) ;
	if (listensocket==-1) {
		fprintf(debugfp,"[%s] Failed to properly open socket descriptor.\n",logTime()) ;
		perror("SOCKET") ;
		exit(EXIT_FAILURE) ;
	} else {
		fprintf(debugfp,"[%s] Opened socket descriptor.\n",logTime()) ;
	}

	socketinfo.sin_family = AF_INET ;
	socketinfo.sin_port = htons(prog->config->listen) ;
	errVal = inet_aton("127.0.0.1",&socketinfo.sin_addr) ;
	if (errVal==0) {
		fprintf(debugfp,"[%s] Error converting 127.0.0.1 to an address structure.\n",logTime()) ;
		exit(EXIT_FAILURE) ;
	} else {
		fprintf(debugfp,"[%s] Converted 127.0.0.1 to address structure.\n",logTime()) ;
	}
	
	errVal = bind(listensocket, (struct sockaddr *)&socketinfo, sizeof(struct sockaddr));
	if (errVal!=0) {
		fprintf(debugfp,"[%s] Failed to bind socket to address and port.\n",logTime()) ;
		perror("BIND") ;
		exit(EXIT_FAILURE) ;
	} else {
		fprintf(debugfp,"[%s] Bound socket to 127.0.0.1 on port %i.\n",logTime(),prog->config->listen) ;
	}

	errVal = listen(listensocket, BACKLOG) ;
	if (errVal!=0) {
		fprintf(debugfp,"[%s] Failed to set socket to listen.\n",logTime()) ;
		perror("LISTEN") ;
		exit(EXIT_FAILURE) ;
	} else {
		fprintf(debugfp,"[%s] Set socket to listen for incomming connections.\n",logTime()) ;
	}
	

	while(1) {
		fprintf(debugfp,"[%s] (Re)Entering listener loop.\n",logTime()) ;

		addrSize = sizeof(struct sockaddr) ;
		prog->connection = accept(listensocket, &clientinfo, &addrSize) ;
		pthread_create(&prog->canceler,NULL,send_cancel,prog) ;
	}
	return NULL ;
}
