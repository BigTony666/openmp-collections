/**********************************
 * DESCRIPTION: A program to find the numbers between 1-1000 which can be divisible by 3 or 4 using pthreads and semaphore
 *
 * Author: Kejie Zhang
 * LAST UPDATED: 02/13/2019
 *
 * USAGE: gcc -O3 div_3_4.c -o div_3_4 -pthread
 * USEFUL REFERENCE:
 *    -> Pthreads: https://computing.llnl.gov/tutorials/pthreads/
**********************************/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>


#define MAX_NUM 1000
sem_t sem1, sem2, sem3, sem4;
int x;

void *f1(void *p) {
    int i;
    bool div_3 = false, div_4 = false;
    for (i = 1; i <= MAX_NUM / 4; i++) {
        sem_wait(&sem1);
        if(i % 3 == 0) {
            div_3 = true;
        }
        if(i % 4 == 0) {
            div_4 = true;
        }

        if(div_3) {
            if(div_4) {
                x++;
                printf("\n%d is divisible by 3 and 4", i);
            } else {
                x++;
                printf("\n%d is divisible by 3", i);
            }
        } else if (div_4) {
            x++;
            printf("\n%d is divisible by 4", i);
        } else {
            printf("\n%d is not divisible by 3 or 4", i);
        }
        div_3 = false;
        div_4 = false;
        sem_post(&sem2);
    }
}

void *f2(void *p) {
    int i;
    bool div_3 = false, div_4 = false;
    for (i = MAX_NUM / 4 + 1; i <= MAX_NUM / 2; i++) {
        sem_wait(&sem2);
        if(i % 3 == 0) {
            div_3 = true;
        }
        if(i % 4 == 0) {
            div_4 = true;
        }

        if(div_3) {
            if(div_4) {
                x++;
                printf("\n%d is divisible by 3 and 4", i);
            } else {
                x++;
                printf("\n%d is divisible by 3", i);
            }
        } else if (div_4) {
            x++;
            printf("\n%d is divisible by 4", i);
        } else {
            printf("\n%d is not divisible by 3 or 4", i);
        }
        div_3 = false;
        div_4 = false;
        sem_post(&sem3);
    }
}

void *f3(void *p) {
    int i;
    bool div_3 = false, div_4 = false;
    for (i = MAX_NUM / 2 + 1; i <= MAX_NUM - MAX_NUM / 4; i++) {
        sem_wait(&sem3);
        if(i % 3 == 0) {
            div_3 = true;
        }
        if(i % 4 == 0) {
            div_4 = true;
        }

        if(div_3) {
            if(div_4) {
                x++;
                printf("\n%d is divisible by 3 and 4", i);
            } else {
                x++;
                printf("\n%d is divisible by 3", i);
            }
        } else if (div_4) {
            x++;
            printf("\n%d is divisible by 4", i);
        } else {
            printf("\n%d is not divisible by 3 or 4", i);
        }
        div_3 = false;
        div_4 = false;
        sem_post(&sem4);
    }
}

void *f4(void *p) {
    int i;
    bool div_3 = false, div_4 = false;
    for (i = MAX_NUM - MAX_NUM / 4 + 1; i <= MAX_NUM; i++) {
        sem_wait(&sem4);
        if(i % 3 == 0) {
            div_3 = true;
        }
        if(i % 4 == 0) {
            div_4 = true;
        }

        if(div_3) {
            if(div_4) {
                x++;
                printf("\n%d is divisible by 3 and 4", i);
            } else {
                x++;
                printf("\n%d is divisible by 3", i);
            }
        } else if (div_4) {
            x++;
            printf("\n%d is divisible by 4", i);
        } else {
            printf("\n%d is not divisible by 3 or 4", i);
        }
        div_3 = false;
        div_4 = false;
        sem_post(&sem1);
    }
}


int main() {
    pthread_t p1, p2, p3, p4;
    x = 0;
    sem_init(&sem1, 0, 1);
    sem_init(&sem2, 0, 0);
    sem_init(&sem3, 0, 0);
    sem_init(&sem4, 0, 0);

    pthread_create(&p1, NULL, f1, (void *) NULL);
    pthread_create(&p2, NULL, f2, (void *) NULL);
    pthread_create(&p3, NULL, f3, (void *) NULL);
    pthread_create(&p4, NULL, f4, (void *) NULL);
    pthread_join(p1, NULL);
    pthread_join(p2, NULL);
    pthread_join(p3, NULL);
    pthread_join(p4, NULL);
    printf("\n x = %d\n", x);

    return 0;
}