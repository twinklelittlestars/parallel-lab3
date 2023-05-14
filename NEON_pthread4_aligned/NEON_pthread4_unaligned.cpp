//��δ���Ҳ�����ö��߳�+neonָ��Ż���ֻ������δ������㷨
#include <iostream>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<sys/time.h>
#include<arm_neon.h>
#define NUM_THREADS 4//��һ�߳���
using namespace std;
const int n = 3000;//�����ģΪ200
//��ʼ������
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
//�߳����ݽṹ����
typedef struct{
    int t_id;//�߳�id
}threadParam_t;
//barrier����
pthread_barrier_t barrier_Division;//�Գ������ֽ��ж��̲߳���
pthread_barrier_t barrier_Elimination;//����ȥ���ֽ��ж��̲߳���
//�̺߳�������
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

        //��һ��ͬ����
        pthread_barrier_wait(&barrier_Division);

        for(int i = k + 1 + t_id;i < n;i += NUM_THREADS){
            //��ȥ
            vaik = vmovq_n_f32(matix[i][k]);
            int j = k + 1;
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
        //�ڶ���ͬ����
        pthread_barrier_wait(&barrier_Elimination);
    }
    pthread_exit(NULL);

}
int main()
{
    //��ʼ��
    setmatix();
    struct timeval head;
    struct timeval tail;
    //��ʼ��barrier
    pthread_barrier_init(&barrier_Division,NULL,NUM_THREADS);
    pthread_barrier_init(&barrier_Elimination,NULL,NUM_THREADS);
    //�����߳�
    pthread_t handles[NUM_THREADS];//������Ӧ��handle
    threadParam_t param[NUM_THREADS];//������Ӧ���߳����ݽṹ
    gettimeofday(&head,NULL);
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        param[t_id].t_id = t_id;
        pthread_create(&handles[t_id],NULL,threadFunc,(void*)&param[t_id]);
    }
    for(int t_id = 0;t_id < NUM_THREADS;++t_id){
        pthread_join(handles[t_id],NULL);
    }
    gettimeofday(&tail,NULL);

    //�������е�barrier
    pthread_barrier_destroy(&barrier_Division);
    pthread_barrier_destroy(&barrier_Elimination);
    cout<<"NUM_THREADS:"<<NUM_THREADS<<" Scale: "<<n<<"  time: "<<(tail.tv_sec-head.tv_sec)*1000.0+(tail.tv_usec-head.tv_usec)/1000.0<<"ms";

    return 0;
}
