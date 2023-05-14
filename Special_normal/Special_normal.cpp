//未进行任何优化的特殊高斯算法
#include <iostream>
#include <fstream>
#include <sstream>
#include<bitset>
#include<windows.h>
using namespace std;
const int Columnnum = 43577;//矩阵列数
const int Rnum = 39477;//消元子的行数
const int Enum = 54274;//被消元行的函数
const int ArrayColumn = 1362;//矩阵中每列所需要的int数组的数量
const int leftbit = 7;
unsigned int R[Columnnum][ArrayColumn];//非零消元子矩阵
unsigned int E[Enum][ArrayColumn];//被消元行矩阵
int First[Enum];//每行非零元素的最小列编号
bool IsNULL[Columnnum] = { 1 };//标记每列是否为零列
bitset<32> MyBit(0);
char fin[90000] = { 0 };//记得根据矩阵列数修改fin大小
int Find_First(int index) {//查找 E 矩阵中的某一行 i 中的第一个非零元素所在的列编号
    int j = 0;
    int cnt = 0;
    while (E[index][j] == 0) {
        j++;
        if (j == ArrayColumn) break;
    }
    if (j == ArrayColumn) return -1;
    unsigned int tmp = E[index][j];
    while (tmp != 0) {
        tmp = tmp >> 1;
        cnt++;
    }
    return Columnnum - 1 - ((j + 1) * 32 - cnt - leftbit);
}
void Init_R() {//从文件中读取非零消元子矩阵 R，将其存储在二维数组 R 中
    unsigned int a;
    ifstream infile("E:\\并行程序\\data\\Groebner\\测试样例10 矩阵列数43577，非零消元子39477，被消元行54274\\消元子.txt");

    int index;
    while (infile.getline(fin, sizeof(fin))) {
        std::stringstream line(fin);
        bool flag = 0;
        while (line >> a) {
            if (flag == 0) {
                index = a;
                flag = 1;
            }
            int k = a % 32;
            int j = a / 32;
            int temp = 1 << k;
            R[index][ArrayColumn - 1 - j] += temp;
        }
    }
}
void Init_E() {//从文件中读取被消元行矩阵 E，将其存储在二维数组 E 中
    unsigned int a;
    ifstream infile("E:\\并行程序\\data\\Groebner\\测试样例10 矩阵列数43577，非零消元子39477，被消元行5427463\\被消元行.txt");

    int index = 0;
    while (infile.getline(fin, sizeof(fin))) {
        std::stringstream line(fin);
        int flag = 0;
        while (line >> a) {
            if (flag == 0) {
                First[index] = a;
                flag = 1;
            }
            int k = a % 32;
            int j = a / 32;
            int temp = 1 << k;
            E[index][ArrayColumn - 1 - j] += temp;
        }
        index++;
    }
}
void Set_R(int eindex, int rindex) {//将 E 矩阵中的某一行 i 赋值给 R 矩阵中的某一行
    for (int j = 0; j < ArrayColumn; j++) {
        R[rindex][j] = E[eindex][j];
    }
}
void Init_IsNULL() {//初始化 IsNULL 数组，标记每列是否为零列
    for (int i = 0; i < Columnnum; i++) {
        bool flag = 0;
        for (int j = 0; j < ArrayColumn; j++) {
            if (R[i][j] != 0) {
                flag = 1;
                IsNULL[i] = 0;
                break;
            }
        }
        if (flag == 0) IsNULL[i] = 1;
    }
}
void XOR(int eindex, int rindex) {//将 E 矩阵中的某一行 i 与 R 矩阵中的某一行 j 进行异或运算
    for (int j = 0; j < ArrayColumn; j++) {
        E[eindex][j] = E[eindex][j] ^ R[rindex][j];
    }
}
void Print() {//Print the answer
    for (int i = 0; i < Enum; i++) {
        cout << "*";
        bool isnull = 1;
        for (int j = 0; j < ArrayColumn; j++) {
            if (E[i][j] != 0) {
                isnull = 0;
                break;
            }
        }
        if (isnull) {
            cout << endl;
            continue;
        }
        for (int j = 0; j < ArrayColumn; j++) {
            if (E[i][j] == 0) continue;
            MyBit = E[i][j];//MENTION: bitset manipulates from the reverse direction
            for (int k = 31; k >= 0; k--) {
                if (MyBit.test(k)) {
                    cout << 32 * (ArrayColumn - j - 1) + k << ' ';
                }
            }
        }
        cout << endl;
    }
}
void Serial() {//程序的主函数，进行串行消元计算
    while (1) {
        bool flag = 1;//flag=0 means all EliminatedLines are handled over
        for (int i = 0; i < Enum; i++) {
            while (First[i] != -1) {
                flag = 0;

                if (IsNULL[First[i]] == 0) {
                    XOR(i, First[i]);
                    First[i] = Find_First(i);
                }
                else {
                    break;
                }
            }
        }
        if (flag == 1) break;
        for (int i = 0; i < Enum; i++) {
            if (First[i] != -1) {
                if (IsNULL[First[i]] == 1) {
                    Set_R(i, First[i]);
                    IsNULL[First[i]] = 0;
                    First[i] = -1;
                    break;
                }
            }
        }
    }

}

int main()
{

    long long head, tail, freq;
    Init_R();
    Init_E();
    Init_IsNULL();

    QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
    QueryPerformanceCounter((LARGE_INTEGER*)&head);
    Serial();
    QueryPerformanceCounter((LARGE_INTEGER*)&tail);
    cout << "Special Gauss, Serial version, Columnnum: " << Columnnum << ", Time: " << (tail - head) * 1000.0 / freq << "ms" << endl;
    //  Print();
    return 0;
}
