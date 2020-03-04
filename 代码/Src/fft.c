#include "arm_math.h"
#include "arm_const_structs.h"
#include "fft.h"
#include "adc.h"
#include "loranode.h"
#define	CNT_LIMIT	11
/*FFT��ض���*/
float fft_inputbuf1[FFT_LENGTH*2];    //FFT��������1
float fft_outputbuf1[FFT_LENGTH];    //FFT�������1

float fft_inputbuf2[FFT_LENGTH*2];    //FFT��������2
float fft_outputbuf2[FFT_LENGTH];    //FFT�������2

float ADC_ConvertedDateWindow1[FFT_LENGTH];		//�Ӵ���Ľ��
float ADC_ConvertedDateWindow2[FFT_LENGTH];

/*���Իع���ض���*/
float arr_IQDif[CNT_LIMIT]={0};	//���ÿ����õ���λ��
int 	arr_MaxNum[CNT_LIMIT]={0};//���ÿ����õ����ֵ���±�

/*����*/
char enableSample_FFT = 0;	//�Ƿ����������FFT���� 0��������   1������

/*���ݼӴ�*/
void windowingSignal()
{
	int i =0;
	for(i = 0;i<FFT_LENGTH;i++)
	{
		ADC_ConvertedDateWindow1[i] = (float)ADC_ConvertedDate1[i] * windowing(i);
		ADC_ConvertedDateWindow2[i] = (float)ADC_ConvertedDate2[i] * windowing(i);
	}
	
}
/*���źŷ�Ϊʵ�����鲿*/
void complexInputSignal()
{
	int i = 0;
	for(i = 0 ; i<FFT_LENGTH ; i++)
	{
		/*ʵ������ż��λ���鲿��������λ*/
		fft_inputbuf1[i*2] = ADC_ConvertedDateWindow1[i];		
		fft_inputbuf2[i*2] = ADC_ConvertedDateWindow2[i];
		/*�鲿ȫ��Ϊ0*/
		fft_inputbuf1[i*2+1] = 0;
		fft_inputbuf2[i*2+1] = 0;
	}     
}  
/*������ ������*/
float windowing(char win)
{
	float result;
  result=(0.5f*(1-arm_cos_f32(2*PI*win/255)));	

  return result;
}

void fftSignal()
{
	/*FFT�任 ����:256�㡢�����������ָ�롢���任��������   */
	arm_cfft_f32(&arm_cfft_sR_f32_len256,fft_inputbuf1,0,1);
	arm_cfft_f32(&arm_cfft_sR_f32_len256,fft_inputbuf2,0,1);
	/*�����ֵ*/
	arm_cmplx_mag_f32(fft_inputbuf1,fft_outputbuf1,FFT_LENGTH);
	arm_cmplx_mag_f32(fft_inputbuf2,fft_outputbuf2,FFT_LENGTH);
}

//������� һ����
double calculatesumx(double x[])
{
    int i;
    double sum=0;
    for (i=0;i<CNT_LIMIT;i++)
    sum=sum+x[i];
    return sum;
}
double calculatesquare(double x[])
{
    int i;
    double sum=0;
    for (i=0;i<CNT_LIMIT;i++)
    sum=sum+x[i]*x[i];
    return sum;
}
double calculatesumxy(double x[],double y[])
{
    int i;	
    double sum=0;
    for (i=0;i<CNT_LIMIT;i++)
    {
      sum=sum+x[i]*y[i];
    }
    return sum;
}
double sumaverage(double x[],double y[])
{
    int i;
    double averagex=0,averagey=0,sum=0;
    for (i=0;i<CNT_LIMIT;i++)
    {
        averagex=averagex+x[i];
    }
    averagex=averagex/CNT_LIMIT;
    for (i=0;i<CNT_LIMIT;i++)
    {
        averagey=averagey+x[i];
    }
    averagey=averagey/CNT_LIMIT;
    for (i=0;i<CNT_LIMIT;i++)
    {
       sum=sum+(x[i]-averagex)*(y[i]-averagey);
    }
    return sum;
}
double squareaverage(double x[])
{
    int i;
    double sum=0,average=0;
    for (i=0;i<CNT_LIMIT;i++)
    average=average+x[i];
    average=average/CNT_LIMIT;
    for (i=0;i<CNT_LIMIT;i++)
    {
        sum=sum+(x[i]-average)*(x[i]-average);
    }
    return sqrt(sum);
}
/*����FFT֮�������*/
void processFFtArr(float *Arr)
{
	int i = 0;
	static int maxCount = 0;	//	��¼�����ֵ�Ĵ���
	float I_Q_Dif = 0;	//��λ��
	float max = Arr[GETMAXSTARTNUM];
	int maxNum = GETMAXSTARTNUM;
	/*��ȡ���ֵ*/
	for(i = GETMAXSTARTNUM;i < (FFT_LENGTH/2) ;i++)
		if(max < Arr[i] && Arr[i] > 1500)
		{
			max = Arr[i];
			maxNum = i;
		}
	/*�ж����ֵ�Ƿ���Ч*/
	if(maxNum > GETMAXSTARTNUM)
	{
		/*��ȡ��λ��*/
		float sin_IQDif = 0;	//sin֮���ֵ
		I_Q_Dif = atan2(fft_inputbuf1[maxNum*2+1],fft_inputbuf1[maxNum*2])-atan2(fft_inputbuf2[maxNum*2+1],fft_inputbuf2[maxNum*2]);//���Ϊ������
		sin_IQDif = arm_sin_f32(I_Q_Dif);	//sin֮��ѱ���+100���-250��ͳһ
		printf("D:%.1f    ",I_Q_Dif*180/PI);	//��Ϊ�Ƕ���
		printf("P:%f  MaxNum:%d\n",sin_IQDif,maxNum);	
		/*�������ֵ�±����λ��Ϣ*/
		arr_MaxNum[maxCount] = maxNum;
		arr_IQDif[maxCount] = sin_IQDif;
		maxCount++;	//���һ����Ч�����ֵ
	}
	/*����ﵽ�˼�¼����------��ʼ���Իع�*/
	if(maxCount==CNT_LIMIT)
	{
		int n1=0;
		int n2=0;
//		for(n1=0;n1<maxCount;n1++)
//			printf("��λ������:%f----��ֵ����:%d\n",arr_IQDif[n1],arr_MaxNum[n1]); 
		/*������*/
		double x[CNT_LIMIT];	
		for(n2=0;n2<CNT_LIMIT;n2++)	
		{
			x[n2]=n2+1;
		}
		/*������*/
		double y[CNT_LIMIT]={0};		 
		for(n2=0;n2<CNT_LIMIT;n2++)
		{
			y[n2]=arr_MaxNum[n2];
		}
    double sumx,sumy,sumxy,sumxx,averagesumxy,squareaveragex,squareaveragey,r,k,b;
    sumx=calculatesumx(x);
    sumy=calculatesumx(y);
    sumxy=calculatesumxy(x,y);
    sumxx=calculatesquare(x);
    b=(sumxx*sumy-sumx*sumxy)/(CNT_LIMIT*sumxx-sumx*sumx);
    k=(CNT_LIMIT*sumxy-sumx*sumy)/(CNT_LIMIT*sumxx-sumx*sumx);
    printf("K=%.4f-----B=%.4f\n",k,b);
    averagesumxy=sumaverage(x,y);
    squareaveragex=squareaverage(x);
    squareaveragey=squareaverage(y);
    r=averagesumxy/(squareaveragex*squareaveragey);
    printf("r:%.4f\n",r);
		/*���*/
		float summ=0;
		for(n2=0;n2<CNT_LIMIT;n2++)
		{
			summ=summ+arr_IQDif[n2];
		}
		/*��ƽ��ֵ*/
		float ave = 0;
		ave = summ/CNT_LIMIT;	
		printf("ave=%f\n",ave);		
		/*�����λƽ��ֵС��-0.9��max���±꼴f��v�𽥼�С----���*/
		if(ave<-0.9)
		{
			//����'y'��ռ��
			strcpy(globalSendMsg,"00000079\r\n");
			node_sendMsg(globalSendMsg);
		}
		else if(ave>0.9&&k>0)
		{
			//����'n'û�б�ռ��
			strcpy(globalSendMsg,"0000006e\r\n");
			node_sendMsg(globalSendMsg);
		}
		else
		{
			HAL_Delay(5000);
		}
		for(n2=0;n2<CNT_LIMIT;n2++)
		{
			arr_IQDif[n2]=0;
			arr_MaxNum[n2]=0;
		}
		maxCount=0;
		
	}
//	printf("------------------\n");
//	printf("Max:%.2f----MaxNum:%d----Frequency:%.0fHz\n",max,maxNum,(float)maxNum*Fs/FFT_LENGTH);
//	if( I_Q_Dif >0 )
//	{
//		printf("Come\n");
//	}
//	else
//	{
//		printf("Go\n");
//	}
//	printf("------------------\n");
}


/*FFT����*/
void FFT_Operate()
{
	windowingSignal();				//�źżӴ�
	complexInputSignal();			//�źŸ�����
	fftSignal();							//����FFT����
}

