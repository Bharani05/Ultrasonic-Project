#include<stdio.h>
#include<stdlib.h>

struct node
{
    int data;
    struct node *next;
};
void addLast(struct node **head,int val)
{
    struct node *newNode = malloc(sizeof(struct node));
    newNode ->data = val;
    newNode ->next = NULL;
    if (*head == NULL)
    *head = newNode;
    else
    {
        struct node *Lastnode = *head;
        while(Lastnode->next !=NULL)
        {

            Lastnode = Lastnode->next;
        }
        Lastnode -> next = newNode;
    }
}
void printlist(struct node *head)
{
    struct node *temp = head;
    while(temp != NULL)
    {
        printf("%d->",temp -> data);
        temp = temp -> next;
    }
    printf("NULL\n");
}
int main()
{
    struct node *head = NULL;
    addLast(&head,13);
    addLast(&head,34);
    addLast(&head,67);

         printlist(head);
         return 0;
}
