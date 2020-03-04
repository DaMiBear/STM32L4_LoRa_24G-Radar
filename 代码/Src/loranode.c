#include "loranode.h"
#include "fft.h"
char sendMsgState = 'n';	//是否允许向LoRa节点发送数据：n不允许  y允许
char joinOrSend = 'j';	//判断是join还是send： j为join		s为send
char globalSendMsg[11] = "0000006e\r\n";	//全局变量，用于存储发送的数据，也便于重新发送	因为使用了strcpy() 所以长度要注意！！ 
/*比较字符串判断串口返回的类型并进行相应操作*/
int node_UartReturn(char * str)
{
	/*LoRa节点刚上电的初始化信息*/
//	if(strcmp(str,"Initialization OK!\r\n")==0)
//	{
//		HAL_Delay(100);
//		/*发送join请求，请求入网*/
//		HAL_UART_Transmit(&huart3,(uint8_t *)"at+join=otaa\r\n",strlen("at+join=otaa\r\n"),0xffff);
//		printf("获得初始化信息，已发送join请求\r\n");
//		joinOrSend = 'j';
//		return 1;
//	}
	/*响应成功信息*/
	if(strcmp(str,"OK\r\n")==0)
	{
		return 2;
	}
	/*入网成功信息STATUS_JOINED_SUCCESS*/
	else if(strcmp(str,"at+recv=3,0,0\r\n")==0)
	{
		//允许采样和FFT计算
		enableSample_FFT = 1;
		//允许单片机向节点发送数据 
		sendMsgState = 'y';	
		printf("获得join请求入网成功信息，允许采样和发送send\r\n");
		return 3;
	}
	/*发送已确认的包成功信息STATUS_TX_COMFIRMED*/
	else if(strcmp(str,"at+recv=1,0,0\r\n")==0)
	{
		//可以继续发送
		sendMsgState = 'y';	
		printf("已经收到发送确认包的成功响应信息，10S后允许继续发送send\r\n");
		HAL_Delay(10000);
		enableSample_FFT = 1;
		printf("已允许采样FFT和发送send\r\n");
		return 4;
	}
	/*加入过程超时，网关无响应 或者 send数据超时 网关无响应(他们返回代码是一样的)*/
	else if(strcmp(str,"at+recv=6,0,0\r\n")==0)
	{
		//如果为join请求
		if(joinOrSend=='j')
		{
			//重新join
			HAL_UART_Transmit(&huart3,(uint8_t *)"at+join=otaa\r\n",strlen("at+join=otaa\r\n"),0xffff);
			joinOrSend = 'j';	//表明刚刚发送的是join
			sendMsgState = 'n';	//入网失败，不能发送send
			printf("网关join超时，已经重新发送join请求\r\n");
		}
		//如果为send
		else if(joinOrSend=='s')
		{
			//重新send
			node_sendMsg(globalSendMsg);
			printf("send超时，已经重新send\r\n");
			return -2;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}
/*向LoRa节点发送信息(注意msg结尾要加上\r\n，且msg为十六进制字符串)*/
void node_sendMsg(char* msg)
{
	if(sendMsgState=='y')
	{
		char* atMsg="at+send=1,8,";	//1:发送确认包	8:端口8
		char* sendMsg=(char*)malloc(strlen(atMsg)+strlen(msg)+1);	//开辟空间储存拼接后的字符串 +1是为了加上结束符 '\0'
		if(sendMsg==NULL)
		{
			return;
		}
		strcpy(sendMsg,atMsg);	//把atMsg放入sendMsg中
		strcat(sendMsg,msg);		//把msg放在sendMsg后面
		HAL_UART_Transmit(&huart3,(uint8_t *)sendMsg,strlen(sendMsg),0xffff);
		joinOrSend = 's';	//表明刚刚发送的是send
		sendMsgState = 'n';		//不能继续发送 等待节点响应信息
		enableSample_FFT = 0;	//采样一组数据之后不允许继续采样和计算
		//释放内存	
		free(sendMsg);
		sendMsg = NULL;
	}
}
