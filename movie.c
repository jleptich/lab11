#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>           
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>


#define FINAL_SCALE 0.001
#define DECREMENTS 0.0018
void create_frame(double xvalue, int count);

int main(int argc, char *argv[]){
    int num_child = 0;
    char name[] = "/memory";
    sem_t *memory = sem_open(name, O_CREAT, 0666,1);
    double *xvalue_hold = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    double *hold_count = mmap(NULL,sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED | MAP_ANONYMOUS,-1,0);
    
    if(xvalue_hold == MAP_FAILED){
        perror("map intialization");
        exit(1);
    }
    if(argc == 2){
        num_child = atoi(argv[1]);
    }
    *xvalue_hold = 1 - DECREMENTS*num_child;
    *hold_count = num_child;
    pid_t pid;
    double xvalue = 1;
    int count = 1;
    
    for(int j = 0; j < num_child; j++){
            pid = fork();
            if(pid == 0){
            xvalue = xvalue - DECREMENTS*j;
            count = count +j;
            //printf("%d\n",getppid());
            break;
            }
        }
    if(pid > 0){
        xvalue = 1 - DECREMENTS*num_child;
    }

    while(xvalue > FINAL_SCALE){
        usleep(2);
        pid_t pid_hold = fork();
        if(pid_hold == 0){
        create_frame(xvalue,count);
        }
        sem_wait(memory);
        *xvalue_hold = *xvalue_hold-DECREMENTS;
        xvalue = *xvalue_hold;
        *hold_count = *hold_count + 1;
        count = *hold_count;
        sem_post(memory);
        waitpid(pid_hold,NULL,0);
        }


    if(pid==0){
        exit(1);
    }
    waitpid(pid,NULL,0);

    printf("Frames have been generated!\n");
    munmap(xvalue_hold, sizeof(int));
    sem_close(memory);
    sem_unlink(name);
    return 0;
}

void create_frame(double xvalue, int count){
    double xcenter = 0.351;
	double ycenter = 0.098;
    char buff1[10];
    char buff2[10];
    char buff3[10];
    char buff4[20];
    snprintf(buff1,10, "%f",xvalue);
    snprintf(buff2,10, "%f",xcenter);
    snprintf(buff3,10, "%f",ycenter);
    snprintf(buff4,20, "mandel%d.jpg",count);
    char *envp[] = {NULL};
    char *argv[] = {"./mandel","-s",buff1 ,"-x", buff2, "-y",buff3 ,"-o", buff4,NULL};

    execve("./mandel",argv, envp);


}