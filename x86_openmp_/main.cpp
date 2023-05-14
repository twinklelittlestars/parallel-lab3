//仅使用#pragma omp for来并行处理内层循环，其使用的划分方式为默认的块划分
#include <iostream>
#include<windows.h>
#include<stdlib.h>
#include <omp.h>
#define NUM_THREADS 7
using namespace std;
const int n = 4000;
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
    long long mhead, mtail, freq;

    setmatix();
    QueryPerformanceCounter((LARGE_INTEGER*)&mhead);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    int i, j, k;
    float tmp;
#pragma omp parallel num_threads(NUM_THREADS),private(i,j,k,tmp)
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
#pragma omp for
        for (i = k + 1; i < n; i++) {
            tmp = matix[i][k];
            for (j = k + 1; j < n; j++) {
                matix[i][j] = matix[i][j] - tmp * matix[k][j];
            }
            matix[i][k] = 0.0;
        }
        //离开for循环时，各个线程默认同步，进入下一行的处理
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&mtail);
    cout << (mtail - mhead) * 1000.0 / freq;

    //Print();
    return 0;
}
