/*UDP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

int port = 5550;
#define BUFF_SIZE 1024

struct sockaddr_in client_list[2]; // client_list[0]: Receiver; client_list[1]: Sender

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

// [IN][OUT] s
int xuLyString(char s[], char text[], char number[])
{
	int indexText = 0, indexNumber = 0;
	for (int i = 0; s[i] != '\0'; i++)
	{
		if ((s[i] > 'a' && s[i] < 'z') || (s[i] > 'A' && s[i] < 'Z'))
		{
			text[indexText++] = s[i];
		}
		else if (s[i] > '0' && s[i] < '9')
		{
			number[indexNumber++] = s[i];
		}
		else{
			strcpy(s, "");
			return 1;
		}
	}

	text[indexText] = '\0';
	number[indexNumber] = '\0';

	strcpy(s, number);
	strcat(s, "\n");
	strcat(s, text);
	s[indexNumber + indexText + 1] = '\0';

	return 0;
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./server PORT\n");
		return 1;
	}

	if (isValidPort(argv[1]))
	{
		printf("Usage: ./server PORT\n\tPORT from 0 to 65535\n");
		return 1;
	}

	int server_sock; /* file descriptors */
	char buff[BUFF_SIZE], text[BUFF_SIZE], number[BUFF_SIZE];
	int bytes_sent, bytes_received;
	struct sockaddr_in server; /* server's address information */
	struct sockaddr_in client; /* client's address information */
	int sin_size;

	// Step 1: Construct a UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Bind address to socket
	server.sin_family = AF_INET;
	server.sin_port = htons(port);			 /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY; /* INADDR_ANY puts your IP address automatically */
	bzero(&(server.sin_zero), 8);				 /* zero the rest of the structure */

	if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}

	// Step 3: Communicate with clients
	while (1)
	{
		strcpy(buff, "");
		strcpy(text, "");
		strcpy(number, "");

		sin_size = sizeof(struct sockaddr_in);

		bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);
		if (strcmp(buff, ">") == 0)
		{
			client_list[1] = client;
			printf("[%s:%d] is sender\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
		}
		else if (strcmp(buff, "<") == 0)
		{
			client_list[0] = client;
			printf("[%s:%d] is receive\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port), buff);
		}
		else
		{
			if (bytes_received < 0)
				perror("\nError: ");
			else
			{
				buff[bytes_received] = '\0';
				if (xuLyString(buff, text, number) == 1)
				{
					printf("Error");
					continue;
				}
				printf("Send %s to [%s:%d]\n", buff, inet_ntoa(client_list[0].sin_addr), ntohs(client_list[0].sin_port));
			}

			bytes_sent = sendto(server_sock, buff, strlen(buff), 0, (struct sockaddr *)client_list, sin_size); /* send to the client welcome message */
			if (bytes_sent < 0)
				printf("\nChua set 1 client lam receiver\n");
		}
	}

	close(server_sock);
	return 0;
}
