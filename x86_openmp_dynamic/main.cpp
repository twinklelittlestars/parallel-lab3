//动态划分的方式
//在#pragma omp for指令中使用了schedule(dynamic,50)来指定动态划分的方式，其中的50表示每个线程被分配的迭代次数的平均值
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
        //#pragma omp for //块划分
        //#pragma omp for schedule(static,1) //循环划分
#pragma omp for schedule(dynamic,50) //动态划分
//#pragma omp for schedule(guided,50) //guided划分
        for (i = k + 1; i < n; i++) {
            tmp = matix[i][k];
            //使用列划分
            //#pragma omp for
            for (j = k + 1; j < n; j++) {
                matix[i][j] = matix[i][j] - tmp * matix[k][j];
            }
            matix[i][k] = 0.0;

        }

        /*#pragma omp single
        for(int i = k + 1;i < n; i++)//这样才能保证正确性
            A[i][k] = 0.0;*/
            //离开for循环时，各个线程默认同步，进入下一行的处理
    }

    QueryPerformanceCounter((LARGE_INTEGER*)&mtail);
    cout << (mtail - mhead) * 1000.0 / freq;

    //Print();
    return 0;
}
