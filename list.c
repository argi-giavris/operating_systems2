#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/times.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <math.h>
#include "list.h"




int exists(Orderedlist *head, int number){ //check if number exists in list
    Orderedlist *temp;
    temp = head;
    while(temp != NULL){
        if(temp->data == number){
            return 0;
        }
        temp = temp->next;
    }
    return 1;
}



Orderedlist *insert(Orderedlist *head, int number){//insert to ordered list

    if(head == NULL){ //empty list

        head = malloc(sizeof(Orderedlist));
        head->next = NULL;
        head->data = number;
        return head;

    }else{
        if(exists(head, number) == 0){//check if exists
            return head;
        }
        Orderedlist *newnode, *currnode, *prevnode;
        newnode = malloc(sizeof(Orderedlist));
        newnode->data = number;
        if(head->data > newnode->data){ //new head list
            newnode->next = head;
            head = newnode;
            return head;
        }

        prevnode = head;
        currnode = head->next;
        while(currnode != NULL && number > currnode->data){
            prevnode = currnode;
            currnode = currnode->next;
        }

        prevnode->next = newnode;
        newnode->next = currnode;
        return head;

    }

}


