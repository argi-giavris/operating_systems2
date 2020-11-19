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
#include <sys/time.h>
#include <sys/select.h>
#include "list.h"
#define READ 0
#define WRITE 1
#define _OPEN_SYS_ITOA_EXT



char *my_itoa(char *dest, int i) {
  sprintf(dest, "%d", i);
  return dest;
}



int maxdescriptor(int *fifos, int num){

    int i, max;
    for(i = 0; i < num; i ++){
        if(i == 0){
            max = fifos[i];
        }
        if(fifos[i] > max){
            max = fifos[i];
        }
    }
    return max;
}

int func(struct pollfd *desc, int num){//check if all children closed their pipes
    int i;
    for(i = 0; i < num; i++){
        if(desc[i].revents != POLLHUP)  
        return 0;
    }
    return 1;
}

#define ITOA(n) my_itoa((char [41]) { 0 }, (n) )



int main(int argc, char *argv[]){

    int lb, ub, numOfChildren, numOfInnerChild, fatherpid;

    lb  = atoi(argv[1]);
    ub = atoi(argv[2]);
    numOfChildren = atoi(argv[3]);
    numOfInnerChild = atoi(argv[5]);
    fatherpid = atoi(argv[6]);
    //printf("Inner: fatherpid=%d\n", fatherpid);
    

    int step, i;
    step = (ub - lb) / numOfChildren;
    
    pid_t pid;
    
    int templb, tempub;
    templb = lb;
    tempub = lb + step;

    
    char **fifoName = malloc(sizeof(char *) * numOfChildren); //hold names for each child pipe


    int *fifos= malloc(sizeof(int)*numOfChildren); //hold file descreptors of pipes
    

    for(i = 0; i < numOfChildren; i++){

        fifoName[i] = malloc((strlen("/tmp/pipe") + 2) * sizeof(char));
        sprintf(fifoName[i], "%s%d", "/tmp/pipe", i); //make pipe names

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
        
        if(pid  == 0){   //child process
           // printf("I am child process with id %d and parent %d \n", getpid() , getppid());
           // printf("I AAAm child  with id %d and parent %d \n", getpid() , getppid());
            //printf("I willll send: %d %d\n", templb, tempub);

            if(execl("/home/argiris/Desktop/operating_systems_erg2/leafChild", 
                     "./leafChild",
                     ITOA(templb), 
                     ITOA(tempub),  
                     ITOA(i),
                     fifoName[i],
                     ITOA(numOfInnerChild),
                     ITOA(fatherpid),
                     NULL) == -1){
               printf("error on execl\n");
               return -1;
            }
            
        }else{
            templb = tempub + 1;    //calculate new range to send to child
            tempub = tempub + step + 1;
            if(tempub > ub){
                tempub = ub;
            }
            
            
            //wait(NULL);
        }
    


    }
    //open children pipe

    for(i = 0; i < numOfChildren; i++){
        if((fifos[i] = open(fifoName[i], O_RDONLY )) < 0){  //argv[2] has the name of pipe sent from parent
            printf("Error opening pipe with parent\n");
            return -1;
        }   

    }
    //printf("Before for\n");

    int nfds, num_open_fds;
    struct pollfd *pfds;
    float time;

    num_open_fds = nfds = numOfChildren;
    pfds = calloc(nfds, sizeof(struct pollfd));

    if(pfds == NULL){
        perror("Calloc ");
        exit(-1);
    }

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
                }
            }



        }
        //sleep(1);
        
    }


     Orderedlist *temp;
    /*temp = list;
    printf("\n");
    while(temp != NULL){
        printf("%d ", temp->data);
        temp = temp->next;
    }
    printf("\n");*/
    
   // open fathers pipe
    int fatherPipe;
   

    if((fatherPipe = open(argv[4], O_WRONLY)) < 0){  //argv[2] has the name of pipe sent from parent
        printf("Error opening pipe with parent\n");
        return -1;
    }
    

    
    temp = list;

    while(temp != NULL){
       
        
        if (write(fatherPipe, &temp->data, sizeof(int)) == -1){
	    	perror("write");
	    	return -1;
	    }
        
        temp = temp->next;
    }
    
    close(fatherPipe);

        return 0;

}