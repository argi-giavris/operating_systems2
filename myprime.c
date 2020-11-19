#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include "list.h"

#define READ 0
#define WRITE 1
#define _OPEN_SYS_ITOA_EXT

int signalsreceived;

void handler(){   //handler signals from leaf children
	signal(SIGUSR1, handler);
	signalsreceived++;
}

int func(struct pollfd *desc, int num){//check if all children closed their pipes
    int i;
    for(i = 0; i < num; i++){
        if(desc[i].revents != POLLHUP)  
        return 0;
    }
    return 1;
}

char *my_itoa(char *dest, int i) {
  sprintf(dest, "%d", i);
  return dest;
}


#define ITOA(n) my_itoa((char [41]) { 0 }, (n) )

int main(int argc, char *argv[]){

    if(argc != 7){
        printf("Wrong number of in-line parameters\n");
        return -1;
    }
    
    int lb, ub, numOfChildren;

    lb = atoi(argv[2]);
    ub = atoi(argv[4]);
    numOfChildren = atoi(argv[6]);
    signalsreceived = 0;
    //printf("Lb = %d Ub = %d Num = %d\n",lb,ub,numOfChildren);
  
    int step;
    step = (ub - lb) / numOfChildren;

    
	
    signal(SIGUSR1, handler);

    int i;
    pid_t pid;
    
    int templb, tempub;
    templb = lb;
    tempub = lb + step;

    
    
    char **fifoName = malloc(sizeof(char *) * numOfChildren); //hold names for each child


    int *fifos= malloc(sizeof(int)*numOfChildren);//hold pipes descreptors
    //printf("I am parent with id %d\n", getpid());


    for(i = 0; i < numOfChildren; i++){

        fifoName[i] = malloc((strlen("/tmp/pipe") + 2) * sizeof(char));
        sprintf(fifoName[i], "%s%d", "/tmp/pipe", i); //make pipe names
        

        //printf("%s\n", fifoName[i]);

		if (mkfifo(fifoName[i],0666) == -1){ //make fifo
            if(errno != EEXIST){
                perror("fifo error");
                return -1;    
            }
		}

        if((pid = fork()) < 0){
            perror("failed to fork");
			return -1;
			
        }
        
        if(pid == 0){   //child process
            //printf("I aaam child  with id %d and parent %d \n", getpid() , getppid());
            //printf("I will send: %d %d\n", templb, tempub);

            if(execl("./innerChild", 
                     "./innerChild",
                     ITOA(templb), 
                     ITOA(tempub),  
                     ITOA(numOfChildren),
                     fifoName[i], 
                     ITOA(i),
                     ITOA(getppid()),
                     NULL) == -1){

               perror("Execl");
               exit(-1);
            }
            
        }else{
            templb = tempub + 1;   //calculate range for next child process
            tempub = tempub + step + 1;
            if(tempub > ub){
                tempub = ub;
            }
                       
        }        

    }
    //open fifos
    for(i = 0; i < numOfChildren; i++){
        if((fifos[i] = open(fifoName[i], O_RDONLY )) < 0){  //argv[2] has the name of pipe sent from parent
            printf("Error opening pipe with parent\n");
            return -1;
        }   

    }


    int nfds, num_open_fds;
    struct pollfd *pfds;
    float time;

    num_open_fds = nfds = numOfChildren;
    pfds = calloc(nfds, sizeof(struct pollfd));

    if(pfds == NULL){
        perror("Calloc ");
        exit(-1);
    }
    //initialize poll struct
    for(i = 0; i < numOfChildren; i++){
        pfds[i].fd = fifos[i]; 
        pfds[i].events = POLLIN;
    }

    Orderedlist *list;
    list = NULL;
    
    
    //keep calling poll as long as at lest 1 fd is open
    while(func(pfds, nfds) == 0){
        

        for(i = 0; i < numOfChildren; i++){
            //pfds[i].fd = fifos[i]; 
            pfds[i].events = 0;
            pfds[i].events |= POLLIN;
        }

        int ready,x;
        //printf("About to poll\n");
        ready = poll(pfds, nfds, -1);

        if(ready == -1){
            perror("Poll\n");
            exit(-1);
        }


       // printf("Ready:%d\n", ready);

        for(i = 0; i < nfds; i++){
            
           
            if(pfds[i].revents != 0){

                if(pfds[i].revents & POLLIN){
                    ssize_t s = read(pfds[i].fd, &x, sizeof(int));
                   //ssize_t s2 = read(pfds[i].fd, &time, sizeof(float));
                    if (s == -1)
                        perror("read");
                    if(s > 0)    {
                        //printf("read %zd bytes: %d\n", s, x);
                        
                        list = insert(list, x);
                    }
                        
                       // printf("read %zd bytes: %f\n", s2, time);
                }else if(pfds[i].revents & POLLHUP ){
                   // printf("I Guess desctiptor  %d of child %d closed\n", pfds[i].fd,i );
                   
                }
            }



        }
        //sleep(1);
        
    }

    //printf("In parent\n");
    Orderedlist *temp;
    temp = list;
    printf("\n");
    while(temp != NULL){
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");

    
    while(list != NULL){
        temp = list;
        list = list->next;
        free(temp);
    }
    

    printf("Signals received %d\n", signalsreceived);

    
    for (int i = 0; i < numOfChildren; i++){
		//wait(NULL);		
		if (unlink(fifoName[i]) < 0){
			perror("fifo unlink error");
			return -1;
		}	
	}





}

