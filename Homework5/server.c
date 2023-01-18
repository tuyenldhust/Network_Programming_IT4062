#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#define BACKLOG 2 /* Number of allowed connections */
#define BUFF_SIZE 1024

int listen_sock, conn_sock, port = 5550; /* file descriptors */
char recv_data[BUFF_SIZE], text[BUFF_SIZE], number[BUFF_SIZE];
int bytes_sent, bytes_received;
struct sockaddr_in server; /* server's address information */
struct sockaddr_in client; /* client's address information */
int sin_size;

int validate_port(char *p)
{
	for (int i = 0; p[i] != '\0'; i++)
	{
		if (p[i] < '0' || p[i] > '9')
			return 0;
	}

	port = atoi(p);

	return port > 0 && port < 65535 ? 1 : 0;
}

int xuLyString(char s[], char text[], char number[])
{
	int indexText = 0, indexNumber = 0;
	for (int i = 0; s[i] != '\0'; i++)
	{
		if ((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'))
		{
			text[indexText++] = s[i];
		}
		else if (s[i] >= '0' && s[i] <= '9')
		{
			number[indexNumber++] = s[i];
		}
		else
		{
			strcpy(s, "Error\n");
			return 0;
		}
	}

	text[indexText] = '\0';
	number[indexNumber] = '\0';

	strcpy(s, number);
	strcat(s, "\n");
	strcat(s, text);

	return 1;
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./server PORT\n");
		return 0;
	}

	if (!validate_port(argv[1]))
	{
		printf("Usage: ./server PORT\n\tPORT from 0 to 65535\n");
		return 0;
	}

	// Step 1: Construct a TCP socket to listen connection request
	if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		return 0;
	}

	// Step 2: Bind address to socket
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);							/* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */
	if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
	{ /* calls bind() */
		perror("\nError: ");
		return 0;
	}

	// Step 3: Listen request from client
	if (listen(listen_sock, BACKLOG) == -1)
	{ /* calls listen() */
		perror("\nError: ");
		return 0;
	}

	printf("Server opened!\n");
	// Step 4: Communicate with client
	while (1)
	{
		// accept request
		sin_size = sizeof(struct sockaddr_in);
		if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
			perror("\nError: ");

		// printf("You got a connection from %s:%s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */

		// start conversation
		while (1)
		{
			// receives message from client
			bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
			if (bytes_received <= 0)
			{
				printf("\nConnection closed");
				break;
			}
			else
			{
				recv_data[bytes_received] = '\0';
				if (strcmp(recv_data, "Choice 1") == 0)
				{
					bytes_received = recv(conn_sock, recv_data, BUFF_SIZE - 1, 0); // blocking
					recv_data[bytes_received] = '\0';
					if (bytes_received <= 0)
					{
						printf("\nConnection closed");
						break;
					}

					xuLyString(recv_data, text, number);

					bytes_sent = send(conn_sock, recv_data, strlen(recv_data), 0); /* send to the client welcome message */
					if (bytes_sent <= 0)
					{
						printf("\nConnection closed");
						break;
					}
				}
				else
				{
					int n;
					char data[BUFF_SIZE] = {0};
					while (1)
					{
						// Receive data from client and show it on screen
						n = recv(conn_sock, data, BUFF_SIZE, 0);
						if (n <= 0)
						{
							break;
						}
						else
						{
							if (strcmp(data, "EOF") == 0)
							{
								break;
							}
							else
							{
								printf("%s", data);
								strcpy(data, "");
							}
						}
					}
				}
			}
		}
		close(conn_sock);
		close(listen_sock);
		return 1;
	}
}