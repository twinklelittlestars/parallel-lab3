//AVX，未优化的状态
//采用多线程编程：静态线程+barrier同步，除法阶段按列进行了划分，消去阶段按行进行了划分
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>
#include<immintrin.h>
#include<windows.h>
#include<stdlib.h>
#pragma warning(default:4716)
#pragma comment(lib, "pthreadVC2.lib")
#define NUM_THREADS 1
using namespace std;
const int n = 4000;
float matix[4000][4000];
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
}threadParam_t;
//barrier定义
pthread_barrier_t barrier_Division;
pthread_barrier_t barrier_Elimination;
//线程函数定义
void* threadFunc(void* param) {
    threadParam_t* p = (threadParam_t*)param;
    int t_id = p->t_id;

    for (int k = 0; k < n; ++k) {
        int j = k + t_id + 1;
        for (; j < n; j += NUM_THREADS) {
            matix[k][j] = matix[k][j] / matix[k][k];
        }

        //第一个同步点
        pthread_barrier_wait(&barrier_Division);

        for (int i = k + 1 + t_id; i < n; i += NUM_THREADS) {
            //消去
            for (int j = k + 1; j < n; ++j) {
                matix[i][j] = matix[i][j] - matix[i][k] * matix[k][j];
            }
            matix[i][k] = 0.0;
        }
        //第二个同步点
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);

}
long long head, tail, freq;
void timestart()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
}
void timestop()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout << ((tail - head) * 1000.0 / freq) << endl;


}
int main()
{
    setmatix();
    //初始化barrier
    pthread_barrier_init(&barrier_Division, NULL, NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination, NULL, NUM_THREADS);
    //创建线程
    pthread_t handles[NUM_THREADS];//创建对应的handle
    threadParam_t param[NUM_THREADS];//创建对应的线程数据结构

    timestart();
    for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id], NULL, threadFunc, (void*)&param[t_id]);
    }
    for (int t_id = 0; t_id < NUM_THREADS; ++t_id) {
        pthread_join(handles[t_id], NULL);
    }
    timestop();
    cout << "线程数：" << NUM_THREADS << "矩阵规模：" << n;
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);

    return 0;
}
