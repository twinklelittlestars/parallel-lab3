//未结合SIMD并行算法，只是实现了一种的多线程编程
//动态线程版本
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
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
typedef struct {
    int t_id;//线程id
    int k;
}threadParam_t;
void* threadFunc(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int k = p->k;
    int t_id = p->t_id;
    int i = k + t_id + 1;
    for (; i < n; i += NUM_THREADS) {
        for (int j = k + 1; j < n; j++) {
            matix[i][j] = matix[i][j] - matix[i][k] * matix[k][j];
        }
        matix[i][k] = 0;
    }
    pthread_exit(NULL);
}
int main()
{
    setmatix();
    struct timeval head;
    struct timeval tail;
    gettimeofday(&head, NULL);
    for (int k = 0; k < n; k++) {
        for (int j = k + 1; j < n; j++) {
            matix[k][j] = matix[k][j] / matix[k][k];
        }
        matix[k][k] = 1.0;
        //int worker_count = n - 1 - k;
        pthread_t* handles = new pthread_t[NUM_THREADS];
        threadParam_t* param = new threadParam_t[NUM_THREADS];
        for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
            param[t_id].k = k;
            param[t_id].t_id = t_id;
        }

        for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
            pthread_create(&handles[t_id], NULL, threadFunc, (void*)&param[t_id]);
        }
        for (int t_id = 0; t_id < NUM_THREADS; t_id++) {
            pthread_join(handles[t_id], NULL);
        }

    }
    gettimeofday(&tail, NULL);
    cout <<"线程数:"<<NUM_THREADS<< "问题规模：" << n << " Time: " << (tail.tv_sec - head.tv_sec) * 1000.0 + (tail.tv_usec - head.tv_usec) / 1000.0 << "ms";
    return 0;
}
