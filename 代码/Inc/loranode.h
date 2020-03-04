#ifndef __LORANODE_H
#define __LORANODE_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32l4xx_hal.h"
#include "usart.h"
int node_UartReturn(char* str);
void node_sendMsg(char* msg);
extern char globalSendMsg[11];
extern char joinOrSend;
extern char sendMsgState;
#endif
