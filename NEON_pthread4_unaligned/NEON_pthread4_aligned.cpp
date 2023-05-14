//在多线程的基础上加入了NEON指令集优化来进一步提高性能,NEON指令集是ARM处理器提供的一种SIMD指令集，能够在一个时钟周期内对多个数据进行并行计算，可以极大地提高向量计算的效率。第二段代码中使用了NEON指令集优化的代码来加速矩阵消元的过程，从而提高了程序的执行效率
//也是按列划分
//值得注意的是这段代码进行了向量对齐的操作
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#include<arm_neon.h>
#define NUM_THREADS 4
using namespace std;
const int n = 3000;
//初始化矩阵A
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
const int MAXN=10010;
typedef struct{
    int t_id;//线程id
}threadParam_t;
//barrier定义
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;
//线程函数定义
void *threadFunc(void *param){
    float32x4_t vx = vmovq_n_f32(0);
    float32x4_t vaij = vmovq_n_f32(0);
    float32x4_t vaik = vmovq_n_f32(0);
    float32x4_t vakj = vmovq_n_f32(0);
    threadParam_t *p = (threadParam_t*)param;
    int t_id = p->t_id;

    for(int k = 0;k < n;++k){
        int j = k + t_id + 1;
        for(;j < n; j += NUM_THREADS){
            matix[k][j] = matix[k][j] / matix[k][k];
        }
        matix[k][k] = 1.0;

        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        for(int i = k + 1 + t_id;i < n;i += NUM_THREADS){
            //消去
            vaik = vmovq_n_f32(matix[i][k]);
            int j = k + 1;
            while((k * n + j) % 4 != 0){//do the alignment when j % 4 != 0
                matix[i][j] = matix[i][j] - matix[k][j] * matix[i][k];
                j++;
            }
            for(;j + 4 <= n;j += 4){
                vakj = vld1q_f32(&matix[k][j]);
                vaij = vld1q_f32(&matix[i][j]);
                vx = vmulq_f32(vakj,vaik);
                vaij = vsubq_f32(vaij,vx);
                vst1q_f32(&matix[i][j],vaij);
            }
            for(;j < n; j++){
                matix[i][j] = matix[i][j] - matix[k][j] * matix[i][k];
            }
            matix[i][k] = 0.0;
        }
        //第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);

}
int main()
{
    //初始化
   // ReSet();
    setmatix();
    struct timeval head;
    struct timeval tail;
    //初始化barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构
    gettimeofday(&head,NULL);//开始计时
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }

    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);//结束计时
    cout<<"NUM_THREADS  "<<NUM_THREADS<<"Scale:  "<<n<<" Time:  "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";
    //销毁所有的barrier
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);

    return 0;
}
