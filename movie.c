/*
    Lab: Lab 11 Multi-processing
    Author: Joseph Leptich
    Date: 11/23/2026
    Description:
    Generates frames of the mandela at differing scales to create a movie

command line: gcc movie.c -o movie -lpthread -pthread -Wall -Wextra -lm
*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>           
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <math.h>

//Control macros for the scale and the multiplication factor
#define FINAL_SCALE 0.000000001
#define DECREMENTS 0.66069
void create_frame(double xvalue, int count);

int main(int argc, char *argv[]){
    int num_child = 0;
    char name[] = "/memory";
    //initializes semaphore and the two shared memory variables
    sem_t *memory = sem_open(name, O_CREAT, 0666,1);
    double *xvalue_hold = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    int *hold_count = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    
    if(xvalue_hold == MAP_FAILED || hold_count == MAP_FAILED){
        perror("map intialization");
        exit(1);
    }
    //gets the amount of processes desired
    if(argc == 2){
        num_child = atoi(argv[1]);
    }

    //Value of the xscale after the initial processes were created
    *xvalue_hold = 1*pow(DECREMENTS, (double)num_child);
    *hold_count = num_child+1;
    pid_t pid;
    double xvalue = 1;
    int count = 1;
    
    //Makes all of the initial processes, only children are from the parent
    //Provides the new child created with a new xvalue to scale the mandela image created
    for(int j = 0; j < (num_child-1); j++){
            pid = fork();
            if(pid == 0){
            xvalue = xvalue*pow(DECREMENTS,j);
            count = count + j;
            //printf("%d\n",getppid());
            break;
            }
        }
    if(pid > 0){
        count = num_child;
        xvalue = 1*pow(DECREMENTS,(double)num_child);
    }

    //loop that runs until the scale desired is reach
    while(xvalue > FINAL_SCALE){
        pid_t pid_hold = fork();

        //need to create another child that runs create frame so it can runthe execve function that ends the process
        if(pid_hold == 0){
        create_frame(xvalue,count);
        }
        //Updates the next value for the xscale from the shared memory
        sem_wait(memory);
        *xvalue_hold = *xvalue_hold*DECREMENTS;
        xvalue = *xvalue_hold;
        *hold_count = *hold_count + 1;
        count = *hold_count;
        sem_post(memory);
        waitpid(pid_hold,NULL,0);   //waits for previous process to finish
        }


    if(pid==0){
        exit(1);
    }
    waitpid(pid,NULL,0);

    //clean up function calls
    printf("Frame generation finished!\n");
    munmap(xvalue_hold, sizeof(double));
    munmap(hold_count, sizeof(int));
    sem_close(memory);
    sem_unlink(name);
    return 0;
}

void create_frame(double xvalue, int count){

    //xcenter and ycenter used
    double xcenter = 0.3509991;
	double ycenter = 0.09711805;

    //inputting values into strings, in order to pass those strings into the string array of argv for the 
    //mandel executable program
    char buff1[40];
    char buff2[30];
    char buff3[30];
    char buff4[20];
    snprintf(buff1,40, "%.12f",xvalue);
    snprintf(buff2,30, "%.12f",xcenter);
    snprintf(buff3,30, "%.12f",ycenter);
    snprintf(buff4,20, "mandel%d.jpg",count);
    char *envp[] = {NULL};
    char *argv[] = {"./mandel","-s",buff1 ,"-x", buff2, "-y",buff3 ,"-o", buff4,NULL};

    //runs execve function which runs the mandel program
    execve("./mandel",argv, envp);


}