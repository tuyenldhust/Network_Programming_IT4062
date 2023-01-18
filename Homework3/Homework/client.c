/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int port = 5550;

int isValidIP(char *ip){
  int count_dots = 0, re = 0;
  for (int i = 0; ip[i] != '\0'; i++)
  {
    if (ip[i] == '.'){
			count_dots++;
			continue;
		}
    if (ip[i] < '0' || ip[i] > '9')
      re = 1;
  }

	if (count_dots != 3)
		re = 1;

	return re;
}

int isValidPort(char *p)
{
	for (int i = 0; p[i] != '\0'; i++)
	{
		if (p[i] < '0' || p[i] > '9')
			return 1;
	}

	port = atoi(p);

	return port > 0 && port < 65535 ? 0 : 1;
}

int main(int argc, char const *argv[])
{
	if(argc != 3){
		printf("Usage: %s <Server IP> <Server Port>\n", argv[0]);
		return 1;
	}

	if(isValidIP(argv[1]) || isValidPort(argv[2])){
		printf("Invalid IP or Port!\n");
		return 1;
	}

	int client_sock;
	char buff[BUFF_SIZE];
	struct sockaddr_in server_addr;
	int bytes_sent, bytes_received, sin_size;
	int isContinueSend = 1;

	// Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	sin_size = sizeof(struct sockaddr);

	// Step 3: Communicate with server
	while (1)
	{
		printf("\nDo you want to send? (1: Yes, other: No): ");
		scanf("%d%*c", &isContinueSend);
		if (isContinueSend == 1)
		{
			bytes_sent = sendto(client_sock, ">", sizeof(">"), 0, (struct sockaddr *)&server_addr, sin_size); // Send '>' to server, client is sender
			printf("\nInsert string to send:");
			memset(buff, '\0', (strlen(buff) + 1));
			fgets(buff, BUFF_SIZE, stdin);
			buff[strlen(buff) - 1] = '\0'; // replace '\n' end string with '\0'

			if (strlen(buff) == 0)
				return 0;

			bytes_sent = sendto(client_sock, buff, strlen(buff), 0, (struct sockaddr *)&server_addr, sin_size);
			if (bytes_sent < 0)
			{
				perror("Error: ");
				close(client_sock);
				return 0;
			}
		}
		else
		{
			bytes_sent = sendto(client_sock, "<", sizeof("<"), 0, (struct sockaddr *)&server_addr, sin_size); // Send '<' to server, client is receiver
			bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);
			if (bytes_received < 0)
			{
				perror("Error: ");
				close(client_sock);
				return 0;
			}
			buff[bytes_received] = '\0';
			printf("%s", buff);
		}
	}

	close(client_sock);
	return 0;
}