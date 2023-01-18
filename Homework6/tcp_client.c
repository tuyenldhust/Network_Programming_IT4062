#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define BUFF_SIZE 1024

typedef struct msg_t
{
	enum msg_type
	{
		USER,
		PASS,
		FOUND_USER,
		FOUND_USER_BUT_LOCKED,
		NOT_FOUND_USER,
		PASS_INVALID,
		PASS_VALID,
		LOCKED
	} type;
	char value[BUFF_SIZE];
} Msg;

int main(int argc, char const *argv[])
{
	if (argc != 3)
	{
		printf("Usage: ./client IPAddress PortNumber\n\n");
		return 0;
	}

	int client_sock;
	char buff[BUFF_SIZE + 1];
	struct sockaddr_in server_addr; /* server's address information */
	int msg_len, bytes_sent, bytes_received;

	// Step 1: Construct socket
	client_sock = socket(AF_INET, SOCK_STREAM, 0);

	// Step 2: Specify server address
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);

	// Step 3: Request to connect server
	if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
	{
		printf("\nError!Can not connect to sever! Client exit imediately! ");
		return 0;
	}

	// Step 4: Communicate with server
	Msg msg;

	printf("Please enter username and password\n");
	printf("Username: ");
	scanf("%[^\n]%*c", msg.value);
	msg.type = USER;

	// send username to server
	bytes_sent = send(client_sock, &msg, sizeof(msg), 0);
	if (bytes_sent < 0)
	{
		printf("\nConnection closed!\n");
		return 0;
	}

	bytes_received = recv(client_sock, &msg, sizeof(msg), 0);
	if (bytes_received < 0)
	{
		perror("Error: ");
		return 0;
	}

	switch (msg.type)
	{
	case FOUND_USER:
		break;
	case FOUND_USER_BUT_LOCKED:
		printf("User is locked, now exitting...\n");
		close(client_sock);
		exit(0);
	case NOT_FOUND_USER:
		printf("User not found, now exitting...\n");
		close(client_sock);
		exit(0);
	}

	while (1)
	{
		printf("Password: ");
		scanf("%[^\n]%*c", msg.value);
		msg.type = PASS;

		bytes_sent = send(client_sock, &msg, sizeof(msg), 0);
		if (bytes_sent < 0)
		{
			printf("\nConnection closed!\n");
			return 0;
		}

		bytes_received = recv(client_sock, &msg, sizeof(msg), 0);
		if (bytes_received < 0)
		{
			perror("Error: ");
			return 0;
		}

		switch (msg.type)
		{
		case PASS_VALID:
			printf("Login successfully! Press ENTER to log out and exit\n");
			getchar();
			close(client_sock);
			exit(0);
		case PASS_INVALID:
			printf("Password is invalid. Reinput\n");
			break;
		case LOCKED:
			printf("User is locked, now exitting...\n");
			close(client_sock);
			exit(0);
		default:
			break;
		}
	}
	return 0;
}
