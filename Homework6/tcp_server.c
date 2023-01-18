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

#define BACKLOG 5
#define BUFF_SIZE 1024

typedef struct node_t
{
  char username[BUFF_SIZE];
  char password[BUFF_SIZE];
  int status;
  struct node_t *next;
} Node;

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

Node *head;

/* Handler process signal*/
void sig_chld(int signo)
{
  pid_t pid;
  int stat;

  /* Wait the child process terminate */
  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    printf("Child %d terminated\n", pid);
}

int load_data(char *filename)
{
  int status, count = 0; // number of accounts
  FILE *f;
  char username[BUFF_SIZE], password[BUFF_SIZE];
  Node *current;
  head = current = NULL;

  // open file data
  if ((f = fopen(filename, "r")) == NULL)
  {
    printf("Cannot open file!\n");
    return 0;
  }

  // load accounts to linked list
  while (fscanf(f, "%s %s %d\n", username, password, &status) != EOF)
  {
    // create new node
    Node *node = malloc(sizeof(Node));
    strcpy(node->username, username);
    strcpy(node->password, password);
    node->status = status;

    // add node to list
    if (head == NULL)
      current = head = node;
    else
      current = current->next = node;
    count++;
  }

  fclose(f);
  return 1;
}

Node *find_node(char *username)
{
  Node *current = head;
  while (current != NULL)
  {
    if (strcmp(current->username, username) == 0)
      return current;
    current = current->next;
  }
  return NULL;
}

void save_list(char *filename)
{
  FILE *f;
  f = fopen(filename, "w");
  Node *current;
  for (current = head; current; current = current->next)
    fprintf(f, "%s %s %d\n", current->username, current->password, current->status);
  fclose(f);
}

/*
 * Receive and echo message to client
 * [IN] sockfd: socket descriptor that connects to client
 */
void echo(int sockfd)
{
  int bytes_received, bytes_sent;
  Msg msg;
  Node *found = NULL;

  bytes_received = recv(sockfd, &msg, sizeof(msg), 0); // blocking
  if (bytes_received <= 0)
    printf("Connection closed.");

  found = find_node(msg.value);
  if (found != NULL)
  {
    if (found->status == 1)
      msg.type = FOUND_USER; // username found
    else
      msg.type = FOUND_USER_BUT_LOCKED; // username found but has been locked
  }
  else
    msg.type = NOT_FOUND_USER; // username not found

  // send notification to client
  bytes_sent = send(sockfd, &msg, sizeof(msg), 0); /* echo to the client */
  if (bytes_sent < 0)
    perror("\nError: ");

  int count = 0;

  while (1)
  {
    // receive password
    bytes_received = recv(sockfd, &msg, sizeof(msg), 0);
    if (bytes_received <= 0)
      printf("Connection closed.");

    // validate password
    if (strcmp(found->password, msg.value) == 0)
      msg.type = PASS_VALID; // pass valid
    else
    {
      count++;
      if (count == 3)
      {
        msg.type = LOCKED; // wrong pass 3 times
        found->status = 0; // then lock account
        save_list("account.txt");
      }
      else
        msg.type = PASS_INVALID; // wrong pass < 3 times
    }

    // send notification to client
    bytes_sent = send(sockfd, &msg, sizeof(msg), 0); /* echo to the client */
    if (bytes_sent < 0)
      perror("\nError: ");
  }

  close(sockfd);
}

int main(int argc, char const *argv[])
{
  char filename[] = "account.txt";

  // valid number of argument
  if (argc != 2)
  {
    printf("Usage: ./server PortNumber\n\n");
    return 0;
  }

  // load data from file

  int listen_sock, conn_sock; /* file descriptors */
  struct sockaddr_in server;  /* server's address information */
  struct sockaddr_in client;  /* client's address information */
  pid_t pid;
  int sin_size;

  if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  { /* calls socket() */
    printf("socket() error\n");
    return 0;
  }

  bzero(&server, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(atoi(argv[1]));
  server.sin_addr.s_addr = htonl(INADDR_ANY); /* INADDR_ANY puts your IP address automatically */

  if (bind(listen_sock, (struct sockaddr *)&server, sizeof(server)) == -1)
  {
    perror("\nError: ");
    return 0;
  }

  if (listen(listen_sock, BACKLOG) == -1)
  {
    perror("\nError: ");
    return 0;
  }

  /* Establish a signal handler to catch SIGCHLD */
  signal(SIGCHLD, sig_chld);

  while (1)
  {
    sin_size = sizeof(struct sockaddr_in);
    if ((conn_sock = accept(listen_sock, (struct sockaddr *)&client, &sin_size)) == -1)
    {
      perror("\nError: ");
      return 0;
    }

    printf("You got a connection from %s\n", inet_ntoa(client.sin_addr)); /* prints client's IP */
                                                                          // echo(conn_sock);

    /* For each client, fork spawns a child, and the child handles the new client */
    pid = fork();

    /* fork() is called in child process */
    if (pid < 0)
    {
      perror("\nError: ");
      return 0;
    }
    else if (pid == 0)
    {
      if (load_data(filename) == 0)
        exit(1);
      echo(conn_sock);
    }
    else
    {
      /* The parent closes the connected socket since the child handles the new client */
      close(conn_sock);
    }
  }
  close(listen_sock);
  return 0;
}