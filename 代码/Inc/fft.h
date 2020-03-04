#ifndef __FFT_H
#define __FFT_H
#include "arm_math.h"
#include "main.h"

void fftTest(void) ;

void processFFtArr(float *Arr);
void windowingSignal(void);				//���ݼӴ�
void complexInputSignal(void);		//�����źű�Ϊ����
float windowing(char win);				//�Ӵ�����
void fftSignal(void);							//FFT����
void FFT_Operate(void);						//FFT����

/*���Իع����*/
double calculatesumx(double x[]);
double calculatesquare(double x[]);
double calculatesumxy(double x[],double y[]);
double sumaverage(double x[],double y[]);
double squareaverage(double x[]);

#define PI2     6.28318530717959
#define Fs      1000									//����Ƶ��
#define FFT_LENGTH        256         //FFT����
#define GETMAXSTARTNUM			5					//ȡ��ֵʱ��ʼ����ʼλ��

extern float fft_inputbuf1[FFT_LENGTH*2];    //FFT��������1
extern float fft_outputbuf1[FFT_LENGTH];    //FFT�������1

extern float fft_inputbuf2[FFT_LENGTH*2];    //FFT��������2
extern float fft_outputbuf2[FFT_LENGTH];    //FFT�������2

extern float ADC_ConvertedDateWindow1[FFT_LENGTH];		//�Ӵ���Ľ��
extern float ADC_ConvertedDateWindow2[FFT_LENGTH];

extern char enableSample_FFT;	//�Ƿ����������FFT����
#endif
