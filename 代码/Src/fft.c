#include "arm_math.h"
#include "arm_const_structs.h"
#include "fft.h"
#include "adc.h"
#include "loranode.h"
#define	CNT_LIMIT	11
/*FFT相关定义*/
float fft_inputbuf1[FFT_LENGTH*2];    //FFT输入数组1
float fft_outputbuf1[FFT_LENGTH];    //FFT输出数组1

float fft_inputbuf2[FFT_LENGTH*2];    //FFT输入数组2
float fft_outputbuf2[FFT_LENGTH];    //FFT输出数组2

float ADC_ConvertedDateWindow1[FFT_LENGTH];		//加窗后的结果
float ADC_ConvertedDateWindow2[FFT_LENGTH];

/*线性回归相关定义*/
float arr_IQDif[CNT_LIMIT]={0};	//存放每次求得的相位差
int 	arr_MaxNum[CNT_LIMIT]={0};//存放每次求得的最大值的下标

/*其他*/
char enableSample_FFT = 0;	//是否允许采样和FFT运算 0：不允许   1：允许

/*数据加窗*/
void windowingSignal()
{
	int i =0;
	for(i = 0;i<FFT_LENGTH;i++)
	{
		ADC_ConvertedDateWindow1[i] = (float)ADC_ConvertedDate1[i] * windowing(i);
		ADC_ConvertedDateWindow2[i] = (float)ADC_ConvertedDate2[i] * windowing(i);
	}
	
}
/*把信号分为实部和虚部*/
void complexInputSignal()
{
	int i = 0;
	for(i = 0 ; i<FFT_LENGTH ; i++)
	{
		/*实部放在偶数位，虚部放在奇数位*/
		fft_inputbuf1[i*2] = ADC_ConvertedDateWindow1[i];		
		fft_inputbuf2[i*2] = ADC_ConvertedDateWindow2[i];
		/*虚部全部为0*/
		fft_inputbuf1[i*2+1] = 0;
		fft_inputbuf2[i*2+1] = 0;
	}     
}  
/*汉宁窗 窗函数*/
float windowing(char win)
{
	float result;
  result=(0.5f*(1-arm_cos_f32(2*PI*win/255)));	

  return result;
}

void fftSignal()
{
	/*FFT变换 参数:256点、输入输出数组指针、正变换、不反向   */
	arm_cfft_f32(&arm_cfft_sR_f32_len256,fft_inputbuf1,0,1);
	arm_cfft_f32(&arm_cfft_sR_f32_len256,fft_inputbuf2,0,1);
	/*计算幅值*/
	arm_cmplx_mag_f32(fft_inputbuf1,fft_outputbuf1,FFT_LENGTH);
	arm_cmplx_mag_f32(fft_inputbuf2,fft_outputbuf2,FFT_LENGTH);
}

//线性拟合 一次幂
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
/*处理FFT之后的数据*/
void processFFtArr(float *Arr)
{
	int i = 0;
	static int maxCount = 0;	//	记录求最大值的次数
	float I_Q_Dif = 0;	//相位差
	float max = Arr[GETMAXSTARTNUM];
	int maxNum = GETMAXSTARTNUM;
	/*获取最大值*/
	for(i = GETMAXSTARTNUM;i < (FFT_LENGTH/2) ;i++)
		if(max < Arr[i] && Arr[i] > 1500)
		{
			max = Arr[i];
			maxNum = i;
		}
	/*判断最大值是否有效*/
	if(maxNum > GETMAXSTARTNUM)
	{
		/*获取相位差*/
		float sin_IQDif = 0;	//sin之后的值
		I_Q_Dif = atan2(fft_inputbuf1[maxNum*2+1],fft_inputbuf1[maxNum*2])-atan2(fft_inputbuf2[maxNum*2+1],fft_inputbuf2[maxNum*2]);//结果为弧度制
		sin_IQDif = arm_sin_f32(I_Q_Dif);	//sin之后把比如+100°和-250°统一
		printf("D:%.1f    ",I_Q_Dif*180/PI);	//变为角度制
		printf("P:%f  MaxNum:%d\n",sin_IQDif,maxNum);	
		/*储存最大值下标和相位信息*/
		arr_MaxNum[maxCount] = maxNum;
		arr_IQDif[maxCount] = sin_IQDif;
		maxCount++;	//求得一次有效的最大值
	}
	/*如果达到了记录次数------开始线性回归*/
	if(maxCount==CNT_LIMIT)
	{
		int n1=0;
		int n2=0;
//		for(n1=0;n1<maxCount;n1++)
//			printf("相位差数组:%f----峰值数组:%d\n",arr_IQDif[n1],arr_MaxNum[n1]); 
		/*横坐标*/
		double x[CNT_LIMIT];	
		for(n2=0;n2<CNT_LIMIT;n2++)	
		{
			x[n2]=n2+1;
		}
		/*纵坐标*/
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
		/*求和*/
		float summ=0;
		for(n2=0;n2<CNT_LIMIT;n2++)
		{
			summ=summ+arr_IQDif[n2];
		}
		/*求平均值*/
		float ave = 0;
		ave = summ/CNT_LIMIT;	
		printf("ave=%f\n",ave);		
		/*如果相位平均值小于-0.9且max的下标即f即v逐渐减小----入库*/
		if(ave<-0.9)
		{
			//发送'y'被占用
			strcpy(globalSendMsg,"00000079\r\n");
			node_sendMsg(globalSendMsg);
		}
		else if(ave>0.9&&k>0)
		{
			//发送'n'没有被占用
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


/*FFT操作*/
void FFT_Operate()
{
	windowingSignal();				//信号加窗
	complexInputSignal();			//信号复数化
	fftSignal();							//进行FFT运算
}

