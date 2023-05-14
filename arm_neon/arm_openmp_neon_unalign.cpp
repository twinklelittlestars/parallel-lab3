//使用了neon指令集优化但是未进行对齐操作的openmp编程
#include <iostream>
#include<sys/time.h>
#include<stdlib.h>
#include <omp.h>
#include<arm_neon.h>
#define NUM_THREADS 1
using namespace std;
const int n = 2000;
float matix[n][n];
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
void Print() {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << matix[i][j] << " ";
        }
        cout << endl;
    }
}
int main()
{
    struct timeval head;
    struct timeval tail;

    setmatix();
    gettimeofday(&head, NULL);
    int i, j, k;
    float tmp;
    float32x4_t vx = vmovq_n_f32(0);
    float32x4_t vaij = vmovq_n_f32(0);
    float32x4_t vaik = vmovq_n_f32(0);
    float32x4_t vakj = vmovq_n_f32(0);
#pragma omp parallel num_threads(NUM_THREADS),private(i,j,k,tmp,vx,vaij,vaik,vakj)
    for (k = 1; k < n; k++) {
        //串行部分，也可以尝试并行化
#pragma omp single
        {
            tmp = matix[k][k];
            for (j = k + 1; j < n; j++) {
                matix[k][j] = matix[k][j] / tmp;
            }
            matix[k][k] = 1.0;
        }
        //并行部分，使用行划分

#pragma omp for private(vx,vaij,vaik,vakj,i,j)
        for (i = k + 1; i < n; i++) {
            vaik = vmovq_n_f32(matix[i][k]);
            int j = k + 1;
            for (; j + 4 <= n; j += 4) {
                vakj = vld1q_f32(&matix[k][j]);
                vaij = vld1q_f32(&matix[i][j]);
                vx = vmulq_f32(vakj, vaik);
                vaij = vsubq_f32(vaij, vx);
                vst1q_f32(&matix[i][j], vaij);
            }
            for (; j < n; j++) {
                matix[i][j] = matix[i][j] - matix[k][j] * matix[i][k];
            }
            matix[i][k] = 0.0;
        }

        //离开for循环时，各个线程默认同步，进入下一行的处理
    }

    gettimeofday(&tail, NULL);
    cout << "N: " << n << " Time: " << (tail.tv_sec - head.tv_sec) * 1000.0 + (tail.tv_usec - head.tv_usec) / 1000.0 << "ms";

    // Print();
    return 0;
}
