#include "genSLL.h"

Account tmpAcc;
int numLoginFail = 0;
char userLogin[25] = {0};

void printData(Node *p)
{
  printf("Account is %s\n", p->ele.active == 1 ? "active" : "blocked");
}

void readData()
{
  FILE *f = fopen("account.txt", "r");
  if (f == NULL)
  {
    printf("Khong the mo file account.txt\n");
    exit(0);
  }

  while (!feof(f))
  {
    fscanf(f, "%s %s %d\n", tmpAcc.username, tmpAcc.password, &tmpAcc.active);
    insertAtHead(tmpAcc);
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
    fprintf(f, "%s %s %d\n", curr->ele.username, curr->ele.password, curr->ele.active);
  }
  fclose(f);
}

void Register()
{
  printf("Account: ");
  scanf("%s%*c", tmpAcc.username);

  Node *curr;
  for (curr = root; curr != NULL; curr = curr->next)
  {
    if (strcmp(curr->ele.username, tmpAcc.username) == 0)
    {
      printf("Account existed\n");
      return;
    }
  }

  printf("Password: ");
  scanf("%s%*c", tmpAcc.password);
  tmpAcc.active = 1;

  insertAtHead(tmpAcc);

  save2Text("account.txt");

  printf("Successful registration\n");
}

void SignIn()
{
  printf("Account: ");
  scanf("%s%*c", tmpAcc.username);

  Node *curr;
  for (curr = root; curr != NULL; curr = curr->next)
  {
    if (strcmp(curr->ele.username, tmpAcc.username) == 0)
    {
      if (strlen(userLogin) > 0)
      {
        printf("Account logged\n");
        return;
      }
      if (curr->ele.active == 0)
      {
        printf("Account is blocked\n");
        return;
      }
      else
      {
        printf("Password: ");
        scanf("%s%*c", tmpAcc.password);
        if (strcmp(curr->ele.password, tmpAcc.password) == 0)
        {
          printf("Hello %s\n", curr->ele.username);
          strcpy(userLogin, curr->ele.username);
          numLoginFail = 0;
          return;
        }
        else
        {
          printf("Password is incorrect");
          numLoginFail++;
          if (numLoginFail == 3)
          {
            printf(". Account is blocked");
            curr->ele.active = 0;
            save2Text("account.txt");
          }
          printf("\n");
          return;
        }
      }
    }
  }

  printf("Cannot find account\n");
}

void Search()
{
  Node *curr;
  if (strlen(userLogin) <= 0)
  {
    printf("Account is not sign in\n");
    return;
  }

  printf("Account: ");
  scanf("%s%*c", tmpAcc.username);

  for (curr = root; curr != NULL; curr = curr->next)
  {
    if (strcmp(curr->ele.username, tmpAcc.username) == 0)
    {
      printData(curr);
      return;
    }
  }

  printf("Cannot find account\n");
}

void SignOut()
{
  if (strlen(userLogin) <= 0)
  {
    printf("Account is not sign in\n");
    return;
  }

  printf("Account: ");
  scanf("%s%*c", tmpAcc.username);

  if (strcmp(userLogin, tmpAcc.username) != 0)
  {
    printf("Not match user logged\n");
    return;
  }
  else
  {
    printf("Goodbye %s\n", userLogin);
    userLogin[0] = '\0';
  }
}

int main(int argc, char const *argv[])
{
  int choice = -1;

  readData();

  while (1)
  {
    printf("USER MANAGEMENT PROGRAM\n"
           "-----------------------------------\n"
           "1. Register\n"
           "2. Sign in\n"
           "3. Search\n"
           "4. Sign out\n"
           "Your choice (1-4, other to quit): ");
    scanf("%d%*c", &choice);

    switch (choice)
    {
    case 1:
      Register();
      break;
    case 2:
      SignIn();
      break;
    case 3:
      Search();
      break;
    case 4:
      SignOut();
      break;
    default:
      Free(root);
      exit(0);
    }
  }

  return 0;
}