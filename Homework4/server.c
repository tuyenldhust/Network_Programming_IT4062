/*UDP Echo Server*/
#include <stdio.h> /* These are the usual header files */
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>
#include "genSLL.h"

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

enum login_status
{
	AUTH,
	UN_AUTH
};

typedef struct _message
{
	enum msg_type type;
	char data[BUFF_SIZE];
} Message;

typedef struct _client
{
	char login_account[BUFF_SIZE];
	struct sockaddr_in client_addr;
	int login_status; // [0: not login yet] & [1: logged in]
	int mode;					// [0: sender] & [1: receiver]
	struct _client *next;
} Client;
Client *head_client;

Account tmp_acc;
int server_port = 5550;
int server_sock; /* file descriptors */
int bytes_sent, bytes_received;
struct sockaddr_in server; /* server's address information */
struct sockaddr_in client; /* client's address information */
int sin_size = sizeof(struct sockaddr_in);

Client *new_client()
{
	Client *new = (Client *)malloc(sizeof(Client));
	new->login_status = UN_AUTH;
	new->next = NULL;
	return new;
}

void add_client(struct sockaddr_in client_addr)
{
	Client *new = new_client();
	new->client_addr = client_addr;
	if (head_client == NULL)
		head_client = new; // if linked list is empty
	else
	{
		Client *tmp = head_client; // assign head to p
		while (tmp->next != NULL)
			tmp = tmp->next; // traverse the list until the last node
		tmp->next = new;	 // Point the previous last node to the new node created.
	}
}

void delete_client(struct sockaddr_in client_addr)
{
	Client *tmp = head_client;
	Client *prev = NULL;
	while (tmp != NULL)
	{
		if (strcmp(inet_ntoa(tmp->client_addr.sin_addr), inet_ntoa(client_addr.sin_addr)) == 0 && ntohs(tmp->client_addr.sin_port) == ntohs(client_addr.sin_port))
		{
			if (prev == NULL)
				head_client = tmp->next;
			else
				prev->next = tmp->next;
			free(tmp);
			break;
		}
		prev = tmp;
		tmp = tmp->next;
	}
}

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

void read_data()
{
	FILE *f = fopen("account.txt", "r");
	if (f == NULL)
	{
		printf("Khong the mo file account.txt\n");
		exit(0);
	}

	while (!feof(f))
	{
		fscanf(f, "%s %s %d\n", tmp_acc.username, tmp_acc.password, &tmp_acc.status);
		tmp_acc.numLoginFail = 0;
		insertAtHead(tmp_acc);
	}

	fclose(f);
}

void save2Text(char *filename)
{
	FILE *f = fopen(filename, "w");
	if (f == NULL)
	{
		printf("Khong the mo file %s\n", filename);
		return;
	}
	Node *curr;
	for (curr = root; curr != NULL; curr = curr->next)
	{
		fprintf(f, "%s %s %d\n", curr->ele.username, curr->ele.password, curr->ele.status);
	}
	fclose(f);
}

// [IN][OUT] s
int xu_ly_string(char s[], char text[], char number[])
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
			strcpy(s, "");
			return 0;
		}
	}

	text[indexText] = '\0';
	number[indexNumber] = '\0';

	strcpy(s, number);
	strcat(s, "\n");
	strcat(s, text);
	s[indexNumber + indexText + 1] = '\0';
	printf("%s\n", s);
	return 1;
}

void catch_ctrl_c_and_exit(int sig)
{
	while (head_client != NULL)
	{
		printf("\nDelete %s:%d", inet_ntoa(head_client->client_addr.sin_addr), ntohs(head_client->client_addr.sin_port));
		delete_client(head_client->client_addr);
	}

	printf("\nBye\n");
	exit(0);
}

void process(void *cli, Message msg, int byte_rcv)
{
	struct sockaddr_in client_addr = *(struct sockaddr_in *)cli;

	if (msg.type == CONNECT)
	{
		printf("Received packet from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		add_client(client_addr);
		return;
	}
	else if(msg.type == DISCONNECT){
		printf("[%s:%d] Disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		delete_client(client_addr);
		return;
	}

	char username[BUFF_SIZE], tmp_password[BUFF_SIZE], password[BUFF_SIZE], text[BUFF_SIZE], number[BUFF_SIZE];
	Node *curr;
	Client *tmp = head_client, *tmp2 = head_client;
	while (tmp != NULL)
	{
		if (strcmp(inet_ntoa(tmp->client_addr.sin_addr), inet_ntoa(client_addr.sin_addr)) == 0 && ntohs(tmp->client_addr.sin_port) == ntohs(client_addr.sin_port))
		{
			break;
		}
		tmp = tmp->next;
	}

	if(msg.type == SELECT_MODE){
		printf("[%s:%d] Select mode %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), msg.data[0] - '0' == 1 ? "Sender" : "Receiver");
		tmp->mode = msg.data[0] - '0';
		return;
	}

	while (bytes_received > 0)
	{
		switch (tmp->login_status)
		{
		case AUTH:
			switch (msg.type)
			{
			case LOGOUT:
				printf("[%s:%d] Bye %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), tmp->login_account);
				strcpy(tmp->login_account, "");
				strcpy(msg.data, "Goodbye ");
				strcat(msg.data, tmp->login_account);
				sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
				tmp->login_status = UN_AUTH;
				return;
			case CHANGE_PASS:
				strcpy(tmp_password, msg.data);
				if(!xu_ly_string(tmp_password, text, number)){
					msg.type = INVALID_TYPE_PASS;
					strcpy(msg.data, "Error");
					sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
					printf("[%s:%d] Invalid type password %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), tmp->login_account);
					return;
				}
				strcpy(password, msg.data);
				for (curr = root; curr != NULL; curr = curr->next)
				{
					if (strcmp(curr->ele.username, tmp->login_account) == 0)
					{
						strcpy(curr->ele.password, password);
						break;
					}
				}
				save2Text("account.txt");
				msg.type = CHANGE_PASS_SUCCESS;
				strcpy(msg.data, tmp_password);
				sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
				printf("[%s:%d] Changed password %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), tmp->login_account);
				return;
			case SEND_TO_RECEIVER:
				tmp2 = head_client;
				int countReceiver = 0;
				while (tmp2 != NULL)
				{
					if (tmp2->mode == 2)
					{
						countReceiver++;
						msg.type = SEND_TO_RECEIVER;
						sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&tmp2->client_addr, sizeof(tmp2->client_addr));
					}
					tmp2 = tmp2->next;
				}
				if (countReceiver == 0)
				{
					msg.type = NO_RECEIVER;
					strcpy(msg.data, "No receiver");
					sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
				}
				else{
					msg.type = SEND_TO_RECEIVER_SUCCESS;
					strcpy(msg.data, "");
					sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
				}
				
				return;
			}
			break;
		case UN_AUTH:
			switch (msg.type)
			{
			case LOGIN:
				strcpy(username, msg.data);
				for (curr = root; curr != NULL; curr = curr->next)
				{
					if (strcmp(curr->ele.username, username) == 0)
					{
						tmp2 = head_client;
						while (tmp2 != NULL)
						{
							if (strcmp(tmp2->login_account, username) == 0)
							{
								msg.type = LOGGED_IN;
								strcpy(msg.data, "Account is logged in");
								sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
								printf("[%s:%d] Account %s is logged in\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);
								return;
							}
							tmp2 = tmp2->next;
						}
						msg.type = ACCOUNT_EXIST;
						sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, sin_size);
						recvfrom(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &sin_size);
						strcpy(password, msg.data + strlen(username) + 1);
						if (strcmp(curr->ele.password, password) == 0)
						{
							if (curr->ele.status == 1)
							{
								msg.type = LOGIN_SUCCESS;
								strcpy(msg.data, "OK");
								sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)cli, sin_size);
								printf("[%s:%d] Hello %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);
								curr->ele.numLoginFail = 0;
								tmp->login_status = AUTH;
								strcpy(tmp->login_account, username);
								return;
							}
							else
							{
								msg.type = ACCOUNT_NOT_READY;
								strcpy(msg.data, "Account is not ready");
								printf("[%s:%d] Account %s is not ready\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);
								sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)cli, sin_size);
								return;
							}
						}
						else
						{
							curr->ele.numLoginFail++;
							if (curr->ele.numLoginFail == 3)
							{
								curr->ele.status = 0;
							}
							save2Text("account.txt");
							msg.type = LOGIN_FAIL;
							strcpy(msg.data, "Not OK");
							printf("[%s:%d] Login %s fail\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);
							sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)cli, sin_size);
							return;
						}
					}
				}
				msg.type = ACCOUNT_NOT_EXIST;
				strcpy(msg.data, "Account not exist");
				printf("[%s:%d] Account %s not exist\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), username);
				sendto(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)cli, sin_size);
				return;
			}
			break;
		}
	}

	printf("Client %s:%d disconnected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

int main(int argc, char const *argv[])
{
	if (argc != 2)
	{
		printf("Usage: ./server PORT\n");
		return 1;
	}

	if ((!is_number(argv[1])) || atoi(argv[1]) > 65535 || atoi(argv[1]) < 0)
	{
		printf("\nInvalid port number\n\n");
		exit(-1);
	}
	server_port = atoi(argv[1]);

	read_data();

	Message msg;
	pthread_t tid;

	signal(SIGINT, catch_ctrl_c_and_exit);

	// Step 1: Construct a UDP socket
	if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{ /* calls socket() */
		perror("\nError: ");
		exit(0);
	}

	// Step 2: Bind address to socket
	server.sin_family = AF_INET;
	server.sin_port = htons(server_port); /* Remember htons() from "Conversions" section? =) */
	server.sin_addr.s_addr = INADDR_ANY;	/* INADDR_ANY puts your IP address automatically */
	bzero(&(server.sin_zero), 8);					/* zero the rest of the structure */

	if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
	{ /* calls bind() */
		perror("\nError: ");
		exit(0);
	}

	printf("Server started at port %d\n", server_port);

	// Step 3: Communicate with clients
	while (1)
	{
		bytes_received = recvfrom(server_sock, &msg, sizeof(msg), 0, (struct sockaddr *)&client, &sin_size);
		process((void *)&client, msg, bytes_received);
	}
	close(server_sock);
	return 0;
}
