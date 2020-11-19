#ifndef LIST_H
#define LIST_H



typedef struct L{
    int data;
    struct L *next;
}Orderedlist;


Orderedlist *insert(Orderedlist *, int );
int exists(Orderedlist *, int );


#endif