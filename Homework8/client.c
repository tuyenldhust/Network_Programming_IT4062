#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <sys/utsname.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char **argv)
{
  if (argc != 3) {
    printf("Usage: %s <IP Address> <Port>\n");
    exit(0);
  }
  

	int client_sock;
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;
	
	//Step 1: Construct socket
	client_sock = socket(AF_INET,SOCK_STREAM,0);
	
	//Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	
	//Step 3: Request to connect server
	if(connect(client_sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) < 0){
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}
		
	//Step 4: Communicate with server			
	//send info OS to server
  struct utsname uts;
  uname(&uts);

  sprintf(buff, "OS: %s %s %s %s %s", uts.sysname, uts.nodename, uts.release, uts.version, uts.machine);
	bytes_sent = send(client_sock, buff, strlen(buff), 0);
	if(bytes_sent < 0)
		perror("\nError: ");
	
  printf("Press any key to exit...");
  getchar();
	//Step 4: Close socket
	close(client_sock);
	return 0;
}
