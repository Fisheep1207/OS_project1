#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>

const int FIFO = 1;
const int RR = 2;
const int PSJF = 3;
const int SJF = 4;
const int ERROR = 0;
const int getMyTime = 333;
const int myPrintk = 334;

typedef struct {
    char p_name[64];
    unsigned int t_ready;
    unsigned int t_exec;
    int pid;
} proc;

void RUN_UNIT(){
    for (volatile unsigned long i = 0; i < 1000000UL; i++)
    ;
    return ;
}
void GET_CPU(int pid, int cpu);
void RUN_child(proc *P, int N, int policy);
void START_proc(int pid);
void IDLE_proc(int pid);
void FORK_proc(proc *cur_P);
int NEXT_proc(proc *P, int N, int policy, int running, int now, int RR_last);
int set_policy(char *tmp);
void INIT_P(proc *P, int N);
int compare (const void * a, const void * b);
int scheduler (proc *P, int N, int policy);



int main(){
    char tmp[32]; int N;
    scanf("%s%d", tmp, &N);
    int policy = set_policy(tmp);
    proc *P;
    P = (proc *)malloc(N * sizeof(proc));
    for (int i = 0 ; i < N ; i++) scanf("%s%u%u", P[i].p_name, &P[i].t_ready, &P[i].t_exec);
    INIT_P(P, N);
    scheduler(P, N, policy);
    return 0;
}



// function 


int set_policy(char *tmp){
    if (!strcmp(tmp, "FIFO")){
        return FIFO;
    }
    else if (!strcmp(tmp, "RR")){
        return RR;
    }
    else if (!strcmp(tmp, "PSJF")){
        return PSJF;
    }
    else if(!strcmp(tmp, "SJF")){
        return SJF;
    }
    return ERROR;
}
void INIT_P(proc *P, int N){
    qsort(P, N, sizeof(proc), compare);
    for (int i = 0 ; i < N ; i ++) P[i].pid = -1;
    return ;
}
int compare (const void * a, const void * b){
  return ((proc *)a)->t_ready - ((proc *)b)->t_ready;
}

int scheduler (proc *P, int N, int policy){
    int my_pid = getpid();
    GET_CPU(my_pid, 0); // MAIN->0 CHILD->1
    START_proc(my_pid);
    RUN_child(P, N, policy);
    return 0;
}


void START_proc(int pid) {
    struct sched_param param;
    param.sched_priority = 0;
    if (sched_setscheduler(pid, SCHED_OTHER, &param) < 0){
        perror("Error in setscheduler!");
        return;
    }
    return;
}
void IDLE_proc(int pid){
    struct sched_param param;
    param.sched_priority = 0;
    if (sched_setscheduler(pid, SCHED_IDLE, &param) < 0){
        perror("Error in setscheduler!");
        return;
    }
    return;
}
void FORK_proc(proc *cur_P){
    int pid;
    pid  = fork();
    if (pid == 0){
        unsigned long start_sec, start_float, end_sec, end_float;
        
        //printf("Start: %d\n", getpid());
        syscall(getMyTime, &start_sec, &start_float);
        for (int i = 0 ; i < cur_P->t_exec; i++){
            RUN_UNIT();
        }
        //printf("End: %d\n", getpid());
        syscall(getMyTime, &end_sec,  &end_float);
        syscall(myPrintk, start_sec, start_float, end_sec, end_float, getpid());
        exit(0);
    }
    cur_P->pid = pid;
    //fprintf(stderr,"here %d\n", pid);
    GET_CPU(pid, 1);
    return;
}
int NEXT_proc(proc *P, int N, int policy, int running, int now, int RR_last){
    int ret = -1;
    if ((policy == SJF || policy == FIFO) && running != -1) return running; // non preemtive
    else{
        if (policy == PSJF){
            for (int i = 0 ; i < N; i ++){
                if (P[i].pid == -1 || P[i].t_exec == 0) continue;
                if (ret == -1) ret = i;
                else if (P[ret].t_exec > P[i].t_exec){
                    ret = i;
                }
            }
        }
        else if (policy == RR){
            if (running == -1){
                for (int i = 0 ; i < N ; i ++){
                    if (P[i].pid != -1 && P[i].t_exec != 0){
                        return i;
                    }
                }
            }
            else{
                if ((now - RR_last) % 500 == 0){
                    ret = (running+1) % N;
                    while(P[ret].pid == -1 || P[ret].t_exec == 0){
                        ret = (ret+1)%N;
                    }
                    return ret;
                }
                else{
                    return running;
                }
            }
        }
        else if (policy == SJF){
            for (int i = 0 ; i < N; i ++){
                if (P[i].pid == -1 || P[i].t_exec == 0) continue;
                if (ret == -1) ret = i;
                else if (P[ret].t_exec > P[i].t_exec){
                    ret = i;
                }
            }
        }
        else if (policy == FIFO){
            for (int i = 0 ; i < N; i ++){
                if (P[i].pid == -1 || P[i].t_exec == 0) continue;
                if (ret == -1) ret = i;
                else if (P[ret].t_ready > P[i].t_ready){
                    ret = i;
                }
            }
        }
    }
    return ret;
}

void GET_CPU(int pid, int cpu){
    cpu_set_t mask; CPU_ZERO(&mask); CPU_SET(cpu, &mask);
    if (sched_setaffinity(pid, sizeof(mask), &mask) < 0) {
        perror("Error in setaffinity!");
        exit(1);
    }
    return;
}

void RUN_child(proc *P, int N, int policy){
    int current_time = 0;
    int running = -1;
    int finish = 0;
    int RR_last = -1;
    while(finish < N){
        //fprintf(stderr, "while\n");
        for (int i = 0 ; i < N; i++){
            if (current_time == P[i].t_ready){
                FORK_proc(&P[i]); // Fork the process at the ready time.
                IDLE_proc(P[i].pid); // After fork, block the proc.
            }
        }
        if (running != -1 && P[running].t_exec == 0){
            finish++;
            waitpid(P[running].pid, NULL, 0);
            printf("%s %d\n", P[running].p_name, P[running].pid);
            fflush(stdin);
            running = -1;
            if (finish == N) break;
        }
        int choose = NEXT_proc(P, N, policy, running, current_time, RR_last);
        if (choose != -1){
            if (choose != running){
                //fprintf(stderr,"running = %d, choose = %d, current_time = %d\n", running, choose, current_time);
                IDLE_proc(P[running].pid);
                START_proc(P[choose].pid);
                running = choose;
                RR_last = current_time;
            }
        }
        RUN_UNIT();
        current_time++;
        if (running != -1){
            P[running].t_exec--;
        }
    }
    return ;
}