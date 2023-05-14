//使用AVX指令集进行优化
#include <iostream>
#include<windows.h>
#include<stdlib.h>
#include <omp.h>
#include<immintrin.h>
#define NUM_THREADS 7
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
    long long mhead, mtail, freq;

    setmatix();
    QueryPerformanceCounter((LARGE_INTEGER*)&mhead);
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    int i, j, k;
    float tmp, temp[8];
    __m256 t1, t2, t3;
#pragma omp parallel num_threads(NUM_THREADS),private(i,j,k,tmp,t1,t2,t3,temp)
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


#pragma omp for private(t1,t2,t3,temp,i,j)
        for (i = k + 1; i < n; i++) {
            temp[0] = temp[1] = temp[2] = temp[3] = temp[4] = temp[5] = temp[6] = temp[7] = matix[i][k];
            t1 = _mm256_loadu_ps(temp);
            j = k + 1;
            for (; j + 16 <= n; j += 4)
            {
                t2 = _mm256_loadu_ps(&matix[k][j]);
                t3 = _mm256_loadu_ps(&matix[i][j]);
                t2 = _mm256_mul_ps(t2, t1);
                t3 = _mm256_sub_ps(t3, t2);
                _mm256_storeu_ps(&matix[i][j], t3);

            }
            for (; j < n; j++)
            {
                matix[i][j] = matix[i][j] - matix[i][k] * matix[k][j];
            }
            matix[i][k] = 0.0;
        }

        //离开for循环时，各个线程默认同步，进入下一行的处理
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&mtail);
    cout << (mtail - mhead) * 1000.0 / freq;

    // Print();
    return 0;
}
