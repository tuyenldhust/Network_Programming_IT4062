#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif

char hostname[100], ip[100], aliasHostName[150], aliasIP[150];

int isDomain(char *param)
{
  int count_dots = 0, re = 0;
  for (int i = 0; param[i] != '\0'; i++)
  {
    if (param[i] == '.')
      continue;
    if (param[i] < '0' || param[i] > '9')
      re = 1;
  }
  return re;
}

int getIP(char *param)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  if ((he = gethostbyname(param)) == NULL)
  {
    printf("Not found information\n");
    exit(0);
  }
  addr_list = (struct in_addr **)he->h_addr_list;
  for (i = 0; addr_list[i] != NULL; i++)
  {
    if (i == 0)
      strcpy(ip, inet_ntoa(*addr_list[i]));
    else
    {
      strcat(aliasIP, inet_ntoa(*addr_list[i]));
      strcat(aliasIP, "\n");
    }
  }

  printf("Official IP: %s\n", ip);
  printf("Alias IP: \n%s", aliasIP);
  return 0;
}

int getHostName(char *param)
{
  struct in_addr addr;
  struct hostent *esu;

  inet_aton(param, &addr);
  esu = gethostbyaddr(((const char *)&addr), sizeof(addr), AF_INET);

  if (esu == NULL)
  {
    printf("Not found information\n");
    exit(0);
  }

  strcpy(hostname, esu->h_name);

  for (int i = 0; esu->h_aliases[i] != NULL; i++)
  {
      strcat(aliasHostName, esu->h_aliases[i]);
      strcat(aliasHostName, "\n");
  }

  printf("Official Hostname: %s\n", hostname);
  printf("Alias Hostname: \n%s", aliasHostName);
  return 0;
}

int main(int argc, char *argv[])
{
  if (argc != 2)
  {
    printf("Usage: ./resolver <hostname or ip>\n");
    exit(0);
  }

  char *tmp = argv[1];
  strcpy(aliasHostName, "");
  strcpy(aliasIP, "");

  if (isDomain(tmp) == 1)
  {
    getIP(tmp);
  }
  else
  {
    getHostName(tmp);
  }
}