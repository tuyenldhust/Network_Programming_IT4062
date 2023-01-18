#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 8192
#define BUFF_SIZE_CLIENT 1024

int client_sock, port = 5550;
char buff[BUFF_SIZE];
struct sockaddr_in server_addr; /* server's address information */
int msg_len, bytes_sent, bytes_received;

int validate_number(char *str)
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

int validate_ip(char *ip)
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
    if (!validate_number(ptr)) // check whether the sub string is holding only number or not
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

int send2Server(char *buff)
{
  bytes_sent = send(client_sock, buff, strlen(buff), 0);
  if (bytes_sent < 0)
  {
    printf("\nConnection closed!\n");
    close(client_sock);
    exit(0);
  }
  return 1;
}

int receiveFromServer()
{
  bytes_received = recv(client_sock, buff, BUFF_SIZE - 1, 0);
  if (bytes_received <= 0)
  {
    printf("\nError!Cannot receive data from sever!\n");
    close(client_sock);
    exit(0);
  }
  return 1;
}

void sendStr2Server()
{
  printf("\nInsert string to send:");
  memset(buff, '\0', (strlen(buff) + 1));
  fgets(buff, BUFF_SIZE, stdin);
  buff[strlen(buff) - 1] = '\0';

  if (strlen(buff) == 0)
  {
    send2Server("Disconnect");
    close(client_sock);
    exit(0);
  }

  send2Server(buff);
}

void sendContentFile2Server()
{
  printf("\nInput filename (file in same directory program): ");
  memset(buff, '\0', (strlen(buff) + 1));
  fgets(buff, BUFF_SIZE, stdin);
  buff[strlen(buff) - 1] = '\0';

  if (strlen(buff) == 0)
  {
    close(client_sock);
    exit(0);
  }

  FILE *fileptr = fopen(buff, "rb");

  if (fileptr == NULL)
  {
    printf("\nFile not found!\n");
    fclose(fileptr);
    return;
  }

  printf("Send file to server...\n");
  char data[BUFF_SIZE_CLIENT] = {0};
  char *re;
  int n;
  printf("Sending file content to client...\n");

  while ((re = fgets(data, BUFF_SIZE_CLIENT, fileptr)) != NULL)
  {
    send2Server(data);
  }

  if(re == NULL) send2Server("EOF");
  fclose(fileptr);
}

int main(int argc, char const *argv[])
{

  if (argc != 3)
  {
    printf("Usage: %s <Server IP> <Server Port>\n", argv[0]);
    return 0;
  }

  char ip[17];
  strcpy(ip, argv[1]);

  if (validate_ip(argv[1]) && validate_port(argv[2]))
  {
    // printf("alo");
    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
      printf("\nError!Can not connect to sever! Client exit imediately!\n");
      return 0;
    }

    // Step 4: Communicate with server
    while (1)
    {
      int choice;
      printf("MENU\n"
             "--------------------\n"
             "1. Gui 1 xau bat ky\n"
             "2. Gui noi dung file\n");
      printf("Nhap lua chon: ");
      scanf("%d%*c", &choice);

      switch (choice)
      {
      case 1:
        send2Server("Choice 1");
        sendStr2Server();
        receiveFromServer();

        printf("%s\n", buff);
        break;
      case 2:
        send2Server("Choice 2");
        sendContentFile2Server();
        break;
      default:
        printf("Vui long nhap lua chon tu 1 den 2\n");
        break;
      }
    }
    close(client_sock);
    return 0;
  }
  else
  {
    printf("Invalid IP or Port!\n");
    return 1;
  }
}