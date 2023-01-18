#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
  char username[25];
  char password[25];
  int status;
  int numLoginFail;
} Account;
typedef Account ElementType;
typedef struct Node
{
  ElementType ele;
  struct Node *next;
} Node;

Node *root = NULL, *cur = NULL, *prev = NULL;

ElementType getData();

Node *makeNewNode(ElementType ele)
{
  Node *new = (Node *)malloc(sizeof(Node));
  new->ele = ele;
  new->next = NULL;
  return new;
}

// void traverseList(Node *root)
// {
//   Node *curr;
//   for (curr = root; curr != NULL; curr = curr->next)
//   {
//     printData(curr);
//   }
// }

void insertAtHead(ElementType ele)
{
  Node *new = makeNewNode(ele);
  new->next = root;
  root = new;
  cur = root;
}

void insertAfterCurrentPos(ElementType ele)
{
  Node *new = makeNewNode(ele);
  if (root == NULL)
  {
    root = new;
    cur = root;
  }
  else if (root != NULL && cur == NULL)
  {
    return;
  }
  else
  {
    new->next = cur->next;
    cur->next = new;
    prev = cur;
    cur = cur->next;
  }
}

void insertBeforeCurrentPos(ElementType ele)
{
  /* Node tmp = root;
     while(tmp != NULL && tmp->next != cur && cur != NULL)
     {
        tmp = tmp->next;
     }
     prev = tmp;
   */

  Node *new = makeNewNode(ele);
  if (root == NULL)
  {
    root = new;
    cur = root;
    prev = NULL;
  }
  else
  {
    new->next = cur;
    if (cur == root)
    {
      root = new;
    }
    else
    {
      prev->next = new;
    }
    cur = new;
  }
}

void Free(Node *root)
{
  Node *to_free = root;
  while (to_free != NULL)
  {
    root = root->next;
    free(to_free);
    to_free = root;
  }
}

void deleteFirstElement()
{
  if (root == NULL)
  {
    printf("Con tro root NULL\n");
    return;
  }

  Node *tmp;
  tmp = root;
  root = root->next;
  cur = root;
  prev = NULL;
  free(tmp);
}

void deleteCurrentElement()
{
  if (cur == NULL)
    return;
  if (cur == root)
    deleteFirstElement();
  else
  {
    prev->next = cur->next;
    free(cur);
    cur = prev->next;
    if(cur == NULL) cur = root;
    /*  cur = root; */ 
  }
}

Node *listReverse(Node *root)
{
  cur = prev = NULL;
  while (root != NULL)
  {
    cur = root;
    root = root->next;
    cur->next = prev;
    prev = cur;
  }
  return prev;
}

Node *insertAtPosition(ElementType ad, int n)
{
  cur = root;
  for (int i = 0; i < n; i++)
  {
    prev = cur;
    cur = cur->next;
  }

  Node *new = (Node *)malloc(sizeof(Node));
  new->ele = ad;
  new->next = cur->next;
  cur->next = new;
  if (root == NULL)
    root = cur;
  if (n == 0)
    insertAtHead(ad);
  prev = cur;
  cur = cur->next;
  return new;
}

Node *deleteAtPosition(int n)
{
  if (root == NULL)
    return root;

  cur = root;
  prev = NULL;
  for (int i = 0; i < n; i++)
  {
    prev = cur;
    cur = cur->next;
  }

  if (cur == root)
  {
    deleteFirstElement(root, cur, prev);
    return root;
  }

  Node *temp = cur;
  prev->next = cur->next;
  cur = cur->next;
  if(cur == NULL){
    cur = root;
    prev = NULL;
  }
  free(temp);
  return root;
}