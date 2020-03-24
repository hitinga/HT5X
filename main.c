/**************************************************************
 *Hitinga HT5X UART Demo code
 *
 *www.hitinga.com
 *
 *Author: HM.Pan
 *2020/3/24
 *************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "porting.h"
#include "crc32.h"
#include "iot_stream.h"
#include "protocol.h"

#define IOT_UART_BANDRATE 115200
#define IOT_UART_DATABITS 8
#define IOT_UART_STOPBITS 1
#define IOT_UART_PARITY   'N'

static const uint8_t rcv_magic[MAGIC_LEN] = {
	MAGIC_HEX_0, MAGIC_HEX_1,
	MAGIC_HEX_2, MAGIC_HEX_3
};

static inline int byte_match(uint8_t c)
{
	uint8_t ch = 0;

	int ret = uart_read_nonblock(&ch, 1 , 1000);
	if(ret<=0){
		return -1;
	}

	return (ch == c ? 1:0);
}

static int find_magic(void)
{
	int i;

	for (i = 0; i < MAGIC_LEN; i++) {
		int ret = byte_match(rcv_magic[i]);
		if(ret <= 0){
			return -1;
		}

		if ((i + 1) == MAGIC_LEN){
			//printf("mach the magic\n");
			return 0;
		}
	}

	return -1;
}

static int get_head_left(unsigned char *buff , int len)
{
	int ret = uart_read(buff , len);
	if(ret != len){
		ERROR_PRINTF("get head left failed , want:%d  ret:%d\n",len , ret);
		return -1;
	}

	return 0;
}

static int get_body(uint8_t *body , uint16_t bodylen)
{
	int ret = uart_read(body , bodylen);
	if(ret != bodylen){
		ERROR_PRINTF("get body failed , want:%d  ret:%d\n",bodylen , ret);
		return -1;
	}

	return 0;
}

static int get_crc(uint8_t *buf)
{
	int ret = uart_read(buf , STREAM_CRC_LEN);
	if(ret != STREAM_CRC_LEN){
		ERROR_PRINTF("get head left failed , want:%d  ret:%d\n",STREAM_CRC_LEN , ret);
		return -1;
	}

	return 0;
}

static void PrivateCtrlHTExample(uint32_t play)
{
    //根据协议控制HT模组 , 以暂停播放为例
    int ret = 0;
	// IOT设备的控制
	// 创建字节流结构
	stream_msg_t * stream = HTIOTStreamCreate(IOT_EQUIP_ID);
	if(stream==NULL){
		ERROR_PRINTF("stream create error\n");
        return ;
	}

	uint8_t * pbuf=NULL;
	uint16_t len=0;

	do{
		// 进行对意图的解析协议
		HTIOTStreamAddNode(stream, ME2HT_CTRL_PLAY, play);

		// 获取字节流
		ret = HTIOTStreamPrintBuf(stream,&pbuf,&len);
		if(ret <0){
			ERROR_PRINTF("stream printbuf error\n");
			break;
		}

		ret = uart_write(pbuf,len);
		if(ret < 0){
			ERROR_PRINTF("stream send error\n");
			break;
		}

	}while(0);

	if(stream)
		HTIOTStreamDelete(stream);
	if(pbuf)
		free(pbuf);    
    
    return;
}

static void UartReadProc(void *params)
{
	uint8_t head[12] = {0};
	uint8_t *streambyte = NULL;
	int streamlen = 0;
	uint16_t bodylen = 0;

	for(;;){
		do{
			//找magic头
			if(find_magic() != 0){
				continue;
			}

			uint32_t *magic = (uint32_t *)&head[0];
			*magic = MAGIC_HEX;

			if(get_head_left(&head[MAGIC_LEN] , sizeof(head)-MAGIC_LEN) != 0){
				break;
			}

			streambyte = &head[10];
			bodylen = (uint16_t)(*((uint16_t*)streambyte));

			/*
			   |<-----head---->|<----body---->|<---crc--->|
			   |<-----12------>|<---bodylen-->|<----4---->|
			   */
			streamlen = sizeof(head) + bodylen + STREAM_CRC_LEN;
			streambyte = OS_MALLOC(sizeof(head) + bodylen + STREAM_CRC_LEN);
			if(streambyte == NULL){
				ERROR_PRINTF("out of memory\n");
				msleep(1000);
				break;
			}

			OS_MEMCPY(streambyte , head , sizeof(head));

			if(get_body(streambyte + sizeof(head), bodylen) != 0){
				break;
			}

			if(get_crc(streambyte + sizeof(head) + bodylen) != 0){
				break;
			}

			stream_msg_t * pstream=NULL;

			IOTProcess(&pstream , streambyte , streamlen);

			if(pstream ) {
				HTIOTStreamDelete(pstream);
			}

		}while(0);

		if(streambyte != NULL){
			OS_FREE(streambyte);
			streambyte = NULL;
		}
	}
}


int main(int argc , char *argv[])
{
	if(uart_init() != 0){
        return -1;
    }
    else{
        DEBUG_PRINTF("uart init success!!!\n");
    }

    if(uart_config(IOT_UART_BANDRATE , IOT_UART_DATABITS , IOT_UART_STOPBITS , IOT_UART_PARITY) != 0){
        return -1;
    }
    else{
        DEBUG_PRINTF("uart config success!!!\n");
    }

    if(create_thread((void *)UartReadProc , NULL) != 0){
        return -1;
    }
    else{
        DEBUG_PRINTF("create thread success!!!\n");
    }

    uint32_t play = 0;

    for(;;){
        msleep(5000);
        PrivateCtrlHTExample(play);
        play = !play;
    }

    return 0;

}
