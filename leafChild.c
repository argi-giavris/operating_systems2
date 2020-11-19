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
#include <stdbool.h>
#define READ 0
#define WRITE 1
#define YES 1
#define NO 0

int prime1(int);
int prime2(int);
int prime3(int, int);


int main(int argc, char *argv[]){


    //printf("HIIII I am leaf child with id %d\n", getpid());
    
    struct tms tb1, tb2;
	double ticspersec = (double) sysconf(_SC_CLK_TCK);
	double t1 = times(&tb1);


    int lb, ub, currChild, fathernum, rootpid;

    lb = atoi(argv[1]);
    ub = atoi(argv[2]);
    currChild = atoi(argv[3]);
    fathernum = atoi(argv[5]);
    rootpid = atoi(argv[6]);

    int fatherPipe;
    //printf("In leaf child %d and pipefather %s\n", getpid(), argv[4]);

    if((fatherPipe = open(argv[4], O_WRONLY)) < 0){  //argv[2] has the name of pipe sent from parent
        perror("leaf pipe\n");
        return -1;
    }
   // printf("Descriptor of father pipe = %d\n", fatherPipe);

  
    
    struct pollfd fdarray [1];
    int rc ;

    int i,j,k;
    j = currChild%3;
    k = 4;
    

    for(i = lb; i <= ub; i++){

        

        if(j == 0){
           if(prime1(i) == YES){
                if (write(fatherPipe, &i, sizeof(int)) == -1){
		    	    perror("write");
		    	    return -1;
		        }   
                //printf("Child %d-%d Wrote number %d\n",fathernum,currChild, i );
                //send turnaround for every number

               // printf("Found prime %d\n", i);
            } 

        }else if(j == 1){
            if(prime2(i) == YES){
                if (write(fatherPipe, &i, sizeof(int)) == -1){
		    	    perror("write");
		    	    return -1;
		        }   
                //printf("Child %d-%d Wrote number %d\n",fathernum,currChild, i );
                //send turnaround for every number

               // printf("Found prime %d\n", i);
            }
        }else{
            if(prime3(i,k) == YES){
                if (write(fatherPipe, &i, sizeof(int)) == -1){
		    	    perror("write");
		    	    return -1;
		        }   
                
            }
        }

            
    }
    close(fatherPipe);    

   
    //send signal to root
    //printf("Sending signal to %d\n", rootpid);
    kill(rootpid, SIGUSR1);

    return 0;
}

int prime1 (int n){
    int i;
    if (n ==1) return (NO);
    for (i=2 ; i<n ; i++)
        if ( n % i == 0) return (NO);
    return (YES);
}


int prime2(int n){
    int i=0 , limitup =0;
    limitup = ( int )(sqrt(( float )n));

    if (n ==1) return (NO);


    for (i=2 ; i <= limitup ; i ++)
        if ( n % i == 0) return (NO);
    return (YES);
}

int power(int x, unsigned int y, int p) 
{ 
    int res = 1;      // Initialize result 
    x = x % p;  // Update x if it is more than or 
                // equal to p 
    while (y > 0) 
    { 
        // If y is odd, multiply x with result 
        if (y & 1) 
            res = (res*x) % p; 
  
        // y must be even now 
        y = y>>1; // y = y/2 
        x = (x*x) % p; 
    } 
    return res; 
} 


bool miillerTest(int d, int n) 
{ 
    // Pick a random number in [2..n-2] 
    // Corner cases make sure that n > 4 
    int a = 2 + rand() % (n - 4); 
  
    // Compute a^d % n 
    int x = power(a, d, n); 
  
    if (x == 1  || x == n-1) 
       return true; 
  
    // Keep squaring x while one of the following doesn't 
    // happen 
    // (i)   d does not reach n-1 
    // (ii)  (x^2) % n is not 1 
    // (iii) (x^2) % n is not n-1 
    while (d != n-1) 
    { 
        x = (x * x) % n; 
        d *= 2; 
  
        if (x == 1)      return false; 
        if (x == n-1)    return YES; 
    } 

    // Return composite 
    return NO; 
} 

int prime3(int n, int k){

    if (n <= 1 || n == 4)  return NO; 
    if (n <= 3) return YES; 
  
    // Find r such that n = 2^d * r + 1 for some r >= 1 
    int d = n - 1; 
    while (d % 2 == 0) 
        d /= 2; 
  
    // Iterate given nber of 'k' times 
    for (int i = 0; i < k; i++) {
         if (!miillerTest(d, n)) 
              return NO; 
    }
    
    return YES;
}