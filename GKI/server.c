#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>

#define PORT_BROADCAST 5556
#define PORT_RESPONSE 6666 /* Port that will be opened */
#define BACKLOG 32         /* Number of allowed connections */
#define BUFF_SIZE 1024

int listen_sock, broadcast_fd, response_fd, send_file_fd; /* file descriptors */
char buff[BUFF_SIZE];
int bytes_sent, bytes_received;
struct sockaddr_in for_broadcast, for_send_file, for_response; /* server's address information */
struct sockaddr_in client;                                     /* client's address information */
int sin_size;
int is_correct_command = 1;

struct file_and_port
{
  char file[100];
  char port[6];
};

int send_file(char *filename, int connfd)
{
  FILE *fp;
  char buff[BUFF_SIZE];
  int bytes_send;

  fp = fopen(filename, "r");
  if (fp == NULL)
  {
    strcpy(buff, "Error\0");
    bytes_send = send(connfd, buff, strlen(buff), 0);
    return 0;
  }
  while (fgets(buff, BUFF_SIZE, fp) != NULL)
  {
    bytes_send = send(connfd, buff, strlen(buff), 0);
    if (bytes_send < 0)
    {
      perror("\nError: ");
      return 0;
    }
  }
  return 1;
}

void thread_start_send_file(void *file_port)
{
  pthread_detach(pthread_self());
  // Step 1: Construct a TCP socket to listen connection request
  if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  { /* calls socket() */
    perror("\nError: ");
    return 0;
  }

  // Step 2: Bind address to socket
  bzero(&for_send_file, sizeof(for_send_file));
  for_send_file.sin_family = AF_INET;
  for_send_file.sin_port = htons(atoi(((struct file_and_port *)file_port)->port)); /* Remember htons() from "Conversions" section? =) */
  for_send_file.sin_addr.s_addr = htonl(INADDR_ANY);                               /* INADDR_ANY puts your IP address automatically */
  if (bind(listen_sock, (struct sockaddr *)&for_send_file, sizeof(for_send_file)) == -1)
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

  sin_size = sizeof(struct sockaddr_in);
  if ((send_file_fd = accept(listen_sock, (struct sockaddr *)&for_send_file, &sin_size)) == -1)
    perror("\nError: ");

  int is_send_success = send_file(((struct file_and_port *)file_port)->file, send_file_fd);
  
  if (is_send_success)
  {
    send_done_cmd(&for_send_file);
  }

  close(send_file_fd);
  close(listen_sock);
  pthread_exit(NULL);
}

void send_invalid_cmd(void *broadcast_inf)
{
  pthread_detach(pthread_self());
  send_cmd(0, broadcast_inf);
  pthread_exit(NULL);
}

void send_done_cmd(void *broadcast_inf)
{
  send_cmd(1, broadcast_inf);
}

void send_cmd(int response, void *server_inf)
{
  // Step 1: Construct socket
  response_fd = socket(AF_INET, SOCK_STREAM, 0);

  // Step 2: Specify server address
  for_response.sin_family = AF_INET;
  for_response.sin_port = htons(6666);
  for_response.sin_addr.s_addr = ((struct sockaddr_in *)server_inf)->sin_addr.s_addr;

  // Step 3: Request to connect server
  if (connect(response_fd, (struct sockaddr *)&for_response, sizeof(struct sockaddr)) < 0)
  {
    printf("\nError!Can not connect to sever! Client exit imediately! ");
    return 0;
  }

  if (response)
  {
    bytes_sent = send(response_fd, "DONE", strlen("DONE"), 0);
    if (bytes_sent <= 0)
    {
      printf("\nConnection closed!\n");
    }
  }
  else
  {
    bytes_sent = send(response_fd, "INVALID COMMAND", strlen("INVALID COMMAND"), 0);
    if (bytes_sent <= 0)
    {
      printf("\nConnection closed!\n");
    }
  }
}

int main()
{
  pthread_t tid;
  struct file_and_port file_port;

  // Step 1: Construct a UDP socket
  if ((broadcast_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
  { /* calls socket() */
    perror("\nError: ");
    exit(0);
  }

  // Step 2: Bind address to socket
  for_broadcast.sin_family = AF_INET;
  for_broadcast.sin_port = htons(PORT_BROADCAST); /* Remember htons() from "Conversions" section? =) */
  for_broadcast.sin_addr.s_addr = INADDR_ANY;     /* INADDR_ANY puts your IP address automatically */
  bzero(&(for_broadcast.sin_zero), 8);            /* zero the rest of the structure */

  if (bind(broadcast_fd, (struct sockaddr *)&for_broadcast, sizeof(struct sockaddr)) == -1)
  { /* calls bind() */
    perror("\nError: ");
    exit(0);
  }

  while (1)
  {
    sin_size = sizeof(struct sockaddr_in);
    bytes_received = recvfrom(broadcast_fd, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&for_broadcast, &sin_size);
    if (bytes_received < 0)
      perror("\nError: ");
    else
    {
      buff[bytes_received] = '\0';
      printf("[%s:%d]: %s", inet_ntoa(for_broadcast.sin_addr), ntohs(for_broadcast.sin_port), buff);
    }

    strcpy(file_port.file, "HelloWorld.txt");
    strcpy(file_port.port, "6000");

    if (is_correct_command)
    {
      pthread_create(&tid, NULL, thread_start_send_file, &file_port);
    }
    else
    {
      pthread_create(&tid, NULL, send_invalid_cmd, &for_broadcast);
    }
  }

  close(broadcast_fd);
  return 0;
}
