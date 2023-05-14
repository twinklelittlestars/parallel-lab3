#include <iostream>
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <stdlib.h>
#include <immintrin.h>
using namespace std;

long long head, tail, freq;
float matix[2048][2048];
void setmatix(int n)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            matix[i][j] = 1.00 / (i + j + 1.00);
        }
    }
}
void LU(int n) {

    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++) {

            matix[i][j] /= matix[i][i];
        }
        matix[i][i] = 1.00;
        for (int k = i + 1; k < n; k++) {

            for (int j = i + 1; j < n; j++)
            {
                matix[k][j] -= matix[k][i] * matix[i][j];
            }
            matix[k][i] = 0.00;
        }
    }
}
void timestart()
{
    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
}
void timestop()
{
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);	// end time
    cout << ((tail - head) * 1000.0 / freq) / 1000 << endl;


}
int main()
{
    setmatix(100);
    timestart();
    LU(100);
    timestop();

    setmatix(300);
    timestart();
    LU(300);
    timestop();

    setmatix(600);
    timestart();
    LU(600);
    timestop();

    setmatix(1024);
    timestart();
    LU(1024);
    timestop();

    setmatix(1200);
    timestart();
    LU(1200);
    timestop();

    setmatix(1400);
    timestart();
    LU(1400);
    timestop();

    setmatix(1600);
    timestart();
    LU(1600);
    timestop();

    setmatix(1800);
    timestart();
    LU(1800);
    timestop();

    setmatix(2000);
    timestart();
    LU(2000);
    timestop();

    setmatix(2024);
    timestart();
    LU(2024);
    timestop();
    return 0;
}
