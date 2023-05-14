//neon编程，对列进行划分并行，并采用多线程方式，且是静态线程+Barrier同步
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#include<arm_neon.h>
#define NUM_THREADS 7//线程数是7
using namespace std;
const int n = 3000;//矩阵规模为2000
//初始化矩阵
float matix[3000][3000];
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
    int t_id;//线程id
}threadParam_t;
//barrier定义
pthread_barrier_t barrier_Division;//对除法部分进行多线程操作
pthread_barrier_t barrier_Elimination;//对消去部分进行多线程操作
//线程函数定义
void *threadFunc(void *param){
    float32x4_t vx = vmovq_n_f32(0);
    float32x4_t vaij = vmovq_n_f32(0);
    float32x4_t vaik = vmovq_n_f32(0);
    float32x4_t vakj = vmovq_n_f32(0);
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;

    for(int k = 0;k < n;++k){//多个工作线程进行除法操作
        int j = k + t_id + 1;
        for(;j < n; j += NUM_THREADS){
             matix[k][j] =  matix[k][j] /  matix[k][k];
        }
         matix[k][k] = 1.0;

        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        //按列进行划分
        for(int i = k + 1;i < n;i ++){
            //消去
            for(int j = k + 1 + t_id;j < n; j += NUM_THREADS){
                 matix[i][j] =  matix[i][j] -  matix[i][k] *  matix[k][j];
            }

        }
        //第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
        if(t_id == 0)
            for(int i = k + 1;i < n; i++)
                  matix[i][k] = 0.0;
    }
    pthread_exit(NULL);

}
int main()
{
    //初始化
    setmatix();
    struct timeval head;
    struct timeval tail;
    //初始化barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构
    gettimeofday(&head,NULL);
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }

    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);
    cout<<"NUM_THREADS:"<<NUM_THREADS<<" Scale: "<<n<<"  Time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms"<<endl;
    //销毁所有的barrier
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);

    return 0;
}
