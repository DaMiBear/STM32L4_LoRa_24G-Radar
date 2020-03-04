#ifndef __FFT_H
#define __FFT_H
#include "arm_math.h"
#include "main.h"

void fftTest(void) ;

void processFFtArr(float *Arr);
void windowingSignal(void);				//数据加窗
void complexInputSignal(void);		//输入信号变为复数
float windowing(char win);				//加窗函数
void fftSignal(void);							//FFT运算
void FFT_Operate(void);						//FFT操作

/*线性回归相关*/
double calculatesumx(double x[]);
double calculatesquare(double x[]);
double calculatesumxy(double x[],double y[]);
double sumaverage(double x[],double y[]);
double squareaverage(double x[]);

#define PI2     6.28318530717959
#define Fs      1000									//采样频率
#define FFT_LENGTH        256         //FFT长度
#define GETMAXSTARTNUM			5					//取最值时开始的起始位置

extern float fft_inputbuf1[FFT_LENGTH*2];    //FFT输入数组1
extern float fft_outputbuf1[FFT_LENGTH];    //FFT输出数组1

extern float fft_inputbuf2[FFT_LENGTH*2];    //FFT输入数组2
extern float fft_outputbuf2[FFT_LENGTH];    //FFT输出数组2

extern float ADC_ConvertedDateWindow1[FFT_LENGTH];		//加窗后的结果
extern float ADC_ConvertedDateWindow2[FFT_LENGTH];

extern char enableSample_FFT;	//是否允许采样和FFT运算
#endif
