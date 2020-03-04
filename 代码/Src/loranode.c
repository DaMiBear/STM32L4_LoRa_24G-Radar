#include "loranode.h"
#include "fft.h"
char sendMsgState = 'n';	//�Ƿ�������LoRa�ڵ㷢�����ݣ�n������  y����
char joinOrSend = 'j';	//�ж���join����send�� jΪjoin		sΪsend
char globalSendMsg[11] = "0000006e\r\n";	//ȫ�ֱ��������ڴ洢���͵����ݣ�Ҳ�������·���	��Ϊʹ����strcpy() ���Գ���Ҫע�⣡�� 
/*�Ƚ��ַ����жϴ��ڷ��ص����Ͳ�������Ӧ����*/
int node_UartReturn(char * str)
{
	/*LoRa�ڵ���ϵ�ĳ�ʼ����Ϣ*/
//	if(strcmp(str,"Initialization OK!\r\n")==0)
//	{
//		HAL_Delay(100);
//		/*����join������������*/
//		HAL_UART_Transmit(&huart3,(uint8_t *)"at+join=otaa\r\n",strlen("at+join=otaa\r\n"),0xffff);
//		printf("��ó�ʼ����Ϣ���ѷ���join����\r\n");
//		joinOrSend = 'j';
//		return 1;
//	}
	/*��Ӧ�ɹ���Ϣ*/
	if(strcmp(str,"OK\r\n")==0)
	{
		return 2;
	}
	/*�����ɹ���ϢSTATUS_JOINED_SUCCESS*/
	else if(strcmp(str,"at+recv=3,0,0\r\n")==0)
	{
		//���������FFT����
		enableSample_FFT = 1;
		//����Ƭ����ڵ㷢������ 
		sendMsgState = 'y';	
		printf("���join���������ɹ���Ϣ����������ͷ���send\r\n");
		return 3;
	}
	/*������ȷ�ϵİ��ɹ���ϢSTATUS_TX_COMFIRMED*/
	else if(strcmp(str,"at+recv=1,0,0\r\n")==0)
	{
		//���Լ�������
		sendMsgState = 'y';	
		printf("�Ѿ��յ�����ȷ�ϰ��ĳɹ���Ӧ��Ϣ��10S�������������send\r\n");
		HAL_Delay(10000);
		enableSample_FFT = 1;
		printf("���������FFT�ͷ���send\r\n");
		return 4;
	}
	/*������̳�ʱ����������Ӧ ���� send���ݳ�ʱ ��������Ӧ(���Ƿ��ش�����һ����)*/
	else if(strcmp(str,"at+recv=6,0,0\r\n")==0)
	{
		//���Ϊjoin����
		if(joinOrSend=='j')
		{
			//����join
			HAL_UART_Transmit(&huart3,(uint8_t *)"at+join=otaa\r\n",strlen("at+join=otaa\r\n"),0xffff);
			joinOrSend = 'j';	//�����ոշ��͵���join
			sendMsgState = 'n';	//����ʧ�ܣ����ܷ���send
			printf("����join��ʱ���Ѿ����·���join����\r\n");
		}
		//���Ϊsend
		else if(joinOrSend=='s')
		{
			//����send
			node_sendMsg(globalSendMsg);
			printf("send��ʱ���Ѿ�����send\r\n");
			return -2;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}
/*��LoRa�ڵ㷢����Ϣ(ע��msg��βҪ����\r\n����msgΪʮ�������ַ���)*/
void node_sendMsg(char* msg)
{
	if(sendMsgState=='y')
	{
		char* atMsg="at+send=1,8,";	//1:����ȷ�ϰ�	8:�˿�8
		char* sendMsg=(char*)malloc(strlen(atMsg)+strlen(msg)+1);	//���ٿռ䴢��ƴ�Ӻ���ַ��� +1��Ϊ�˼��Ͻ����� '\0'
		if(sendMsg==NULL)
		{
			return;
		}
		strcpy(sendMsg,atMsg);	//��atMsg����sendMsg��
		strcat(sendMsg,msg);		//��msg����sendMsg����
		HAL_UART_Transmit(&huart3,(uint8_t *)sendMsg,strlen(sendMsg),0xffff);
		joinOrSend = 's';	//�����ոշ��͵���send
		sendMsgState = 'n';		//���ܼ������� �ȴ��ڵ���Ӧ��Ϣ
		enableSample_FFT = 0;	//����һ������֮��������������ͼ���
		//�ͷ��ڴ�	
		free(sendMsg);
		sendMsg = NULL;
	}
}
