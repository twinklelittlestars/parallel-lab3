//未结合SIMD并行算法，只是实现了一种多线程编程方法
//静态线程+信号量同步版本
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<semaphore.h>
#include<sys/time.h>
#define NUM_THREADS 1
using namespace std;
const int n = 2000;
//初始化矩阵
float matix[2048][2048];
void setmatix()
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matix[i][j] = 1.00 / (i + j + 1.00);
        }
    }
}

//线程数据结构定义
typedef struct{
    int t_id;
}threadParam_t;
sem_t sem_main;
sem_t sem_workerstart[NUM_THREADS];
sem_t sem_workerend[NUM_THREADS];
void *threadFunc(void *param){
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;
    for(int k = 0;k < n; k++){
        sem_wait(&sem_workerstart[t_id]);
        for(int i = k + 1 + t_id; i < n; i += NUM_THREADS){
           for(int j = k + 1;j < n; j++){
               matix[i][j] = matix[i][j] - matix[i][k] * matix[k][j];
           }
        matix[i][k] = 0.0;
    }
    sem_post(&sem_main);
    sem_wait(&sem_workerend[t_id]);
    }
    pthread_exit(NULL);
}
int main()
{
    setmatix();
    struct timeval head;
    struct timeval tail;
    sem_init(&sem_main,0,0);
    for(int i = 0;i < NUM_THREADS; i++){
        sem_init(&sem_workerstart[i],0,0);
        sem_init(&sem_workerend[i],0,0);
    }
    pthread_t handles[NUM_THREADS];
    threadParam_t param[NUM_THREADS];
    gettimeofday(&head,NULL);
    for(int t_id = 0;t_id < NUM_THREADS; t_id++){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }
    for(int k = 0;k <n; k++){
        for(int j = k + 1;j < n; j++){
            matix[k][j] = matix[k][j] / matix[k][k];
        }
        matix[k][k] = 1.0;
        for(int t_id = 0;t_id <NUM_THREADS; t_id++){
            sem_post(&sem_workerstart[t_id]);
        }
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            sem_wait(&sem_main);
        }
        for(int t_id = 0;t_id < NUM_THREADS; t_id++){
            sem_post(&sem_workerend[t_id]);
        }
    }
    for(int t_id = 0;t_id <NUM_THREADS; t_id++){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);
    cout <<"线程数:"<<NUM_THREADS<< "问题规模：" << n << " Time: " << (tail.tv_sec - head.tv_sec) * 1000.0 + (tail.tv_usec - head.tv_usec) / 1000.0 << "ms";
    sem_destroy(&sem_main);
    for(int i = 0;i < NUM_THREADS; i++){
        sem_destroy(&sem_workerstart[i]);
        sem_destroy(&sem_workerend[i]);
    }

    return 0;
}
