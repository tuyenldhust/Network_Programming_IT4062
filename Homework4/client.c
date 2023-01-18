/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define BUFF_SIZE 1024

enum msg_type
{
	CONNECT,
	DISCONNECT,
	SELECT_MODE,
	LOGIN,
	LOGIN_SUCCESS,
	LOGGED_IN,
	LOGIN_FAIL,
	ACCOUNT_NOT_READY,
	ACCOUNT_NOT_EXIST,
	ACCOUNT_EXIST,
	CHANGE_PASS,
	CHANGE_PASS_SUCCESS,
	INVALID_TYPE_PASS,
	SEND_TO_RECEIVER,
	NO_RECEIVER,
	SEND_TO_RECEIVER_SUCCESS,
	LOGOUT
};

typedef struct _message
{
	enum msg_type type;
	char data[BUFF_SIZE];
} Message;

int server_port = 5550;
int client_sock;
char server_ip[100];
struct sockaddr_in server_addr;
int bytes_sent, bytes_received, sin_size, is_login = 0;

int is_number(char *str)
{
	while (*str)
	{
		if (!isdigit(*str))
		{ // if the character is not a number, return false
			return 0;
		}
		str++; // point to next character
	}
	return 1;
}

int is_ip(char *ip)
{ // check whether the IP is valid or not
	int i, num, dots = 0;
	char *ptr;
	if (ip == NULL)
		return 0;
	ptr = strtok(ip, "."); // cut the string using dor delimiter
	if (ptr == NULL)
		return 0;
	while (ptr)
	{
		if (!is_number(ptr)) // check whether the sub string is holding only number or not
			return 0;
		num = atoi(ptr); // convert substring to number
		if (num >= 0 && num <= 255)
		{
			ptr = strtok(NULL, "."); // cut the next part of the string
			if (ptr != NULL)
				dots++; // increase the dot count
		}
		else
			return 0;
	}
	if (dots != 3) // if the number of dots are not 3, return false
		return 0;
	return 1;
}

int sign_in()
{
	Message msg;
	char username[100], password[100];
	printf("Account: ");
	scanf("%[^\n]%*c", username);

	msg.type = LOGIN;
	strcpy(msg.data, username);
	sin_size = sizeof(struct sockaddr_in);
	sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);

	recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);

	if (msg.type != ACCOUNT_NOT_EXIST && msg.type != LOGGED_IN)
	{
		printf("Password: ");
		scanf("%[^\n]%*c", password);
		strcat(msg.data, " ");
		strcat(msg.data, password);

		sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);

		recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);

		if (msg.type == LOGIN_SUCCESS)
		{
			printf("%s\n", msg.data);
			is_login = 1;
		}
		else if (msg.type == LOGIN_FAIL)
		{
			printf("%s\n", msg.data);
			is_login = 0;
		}
		else if (msg.type == ACCOUNT_NOT_READY)
		{
			printf("%s\n", msg.data);
			is_login = 0;
		}
	}
	else
	{
		printf("%s!\n", msg.data);
	}

	return is_login;
}

void catch_ctrl_c_and_exit(int sig)
{
	Message msg;
	msg.type = DISCONNECT;
	sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);
	printf("\nBye\n");
	exit(0);
}

int main(int argc, char const *argv[])
{
	Message msg;
	char buff[BUFF_SIZE];

	if (argc != 3)
	{
		printf("Usage: %s <Server IP> <Server Port>\n", argv[0]);
		return 0;
	}

	strcpy(server_ip, argv[1]);
	if (!is_ip(server_ip))
	{
		printf("\nInvalid IP address\n");
		exit(-1);
	}
	strcpy(server_ip, argv[1]);

	if ((!is_number(argv[2])) || atoi(argv[2]) > 65535 || atoi(argv[2]) < 0)
	{
		printf("\nInvalid port number\n\n");
		exit(-1);
	}

	server_port = atoi(argv[2]);
	signal(SIGINT, catch_ctrl_c_and_exit);

	// Step 1: Construct a UDP socket
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Define the address of the server
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(server_ip);
	sin_size = sizeof(struct sockaddr);

	// Step 3: Communicate with server
	msg.type = CONNECT;
	bytes_sent = sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);

	printf("Connected!\n");

	int choice;
	printf("Select Mode: \n"
				 "1. Sender\n"
				 "2. Receiver\n"
				 "Enter your choice: ");
	scanf("%d%*c", &choice);

	msg.type = SELECT_MODE;
	msg.data[0] = choice + '0';
	sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);

	while (1)
	{
		if (!is_login)
		{
			sign_in();
		}
		else
		{
			if (choice == 1)
			{
				printf("Enter message: ");
				scanf("%[^\n]%*c", buff);

				if (strcmp(buff, "bye") == 0)
				{
					msg.type = LOGOUT;
					sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);
					recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);
					printf("%s\n", msg.data);
					is_login = 0;
				}
				else if (strcmp(buff, "changepass") == 0)
				{
					msg.type = CHANGE_PASS;
					printf("New password: ");
					scanf("%[^\n]%*c", buff);
					strcpy(msg.data, buff);
					sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);
					recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);
					printf("%s\n", msg.data);
				}
				else
				{
					msg.type = SEND_TO_RECEIVER;
					strcpy(msg.data, buff);
					sendto(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, sin_size);
					recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);
					printf("%s\n", msg.data);
				}
			}
			else{
				recvfrom(client_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&server_addr, (unsigned int *)&sin_size);
				printf("%s\n", msg.data);
			}
		}
	}

	close(client_sock);
	return 0;
}