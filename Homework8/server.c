#include <stdio.h>          /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/utsname.h>
#include <stdlib.h>

#define BACKLOG 20   /* Number of allowed connections */
#define BUFF_SIZE 1024

struct sockaddr_in *client; /* client's address information */

/* Receive and echo message to client */
void *echo(void *);

int main(int argc, char **argv)
{
  if (argc != 2) {
    printf("Usage: %s <Port>\n");
    exit(0);
  }
  

	int listenfd, *connfd;
	struct sockaddr_in server; /* server's address information */
	int sin_size;
	pthread_t tid;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /* calls socket() */
		perror("\nError: ");
		return 0;
	}
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;         
	server.sin_port = htons(atoi(argv[1])); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);  /* INADDR_ANY puts your IP address automatically */   

	if(bind(listenfd,(struct sockaddr*)&server, sizeof(server))==-1){ 
		perror("\nError: ");
		return 0;
	}     

	if(listen(listenfd, BACKLOG) == -1){  
		perror("\nError: ");
		return 0;
	}
	
	sin_size=sizeof(struct sockaddr_in);
  client = malloc(sin_size);
	while(1){		
		connfd = malloc(sizeof(int));
		if ((*connfd = accept(listenfd, (struct sockaddr *)client, &sin_size)) ==- 1)
			perror("\nError: ");
				
		printf("New connection from %s:%d\n", inet_ntoa(client->sin_addr), ntohs(client->sin_port)); /* prints client's IP */

		/* For each client, spawns a thread, and the thread handles the new client */
		pthread_create(&tid, NULL, &echo, connfd);	
	}
	
	close(listenfd);
	return 0;
}

void *echo(void *arg){
	int connfd;
	int bytes_sent, bytes_received;
  int port;
	char buff[BUFF_SIZE + 1], ip[BUFF_SIZE], file_name[BUFF_SIZE];
  struct utsname uts;
  pthread_mutex_t lock;

  pthread_mutex_lock(&lock);
  strcpy(ip, inet_ntoa((*client).sin_addr));
  port = ntohs((*client).sin_port);
  pthread_mutex_unlock(&lock);

	connfd = *((int *) arg);
	free(arg);
	pthread_detach(pthread_self());
	
	bytes_received = recv(connfd, buff, BUFF_SIZE, 0); //blocking
	if (bytes_received < 0)
		perror("\nError: ");
	else if (bytes_received == 0)
		printf("Connection closed.");

  buff[bytes_received] = '\0';
	
  sprintf(file_name, "client_%s_%d.log", ip, port);

  FILE *fp = fopen(file_name, "w");

  if (fp == NULL) {
    printf("Error opening file");
    close(connfd);
    exit(1);
  }

  fprintf(fp, "IP: %s\nPort: %d\n%s", ip, port, buff);
  fclose(fp);

	close(connfd);	
}