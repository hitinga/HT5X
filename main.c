#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "porting.h"
#include "crc32.h"
#include "iot_stream.h"
#include "protocol.h"

static const uint8_t rcv_magic[MAGIC_LEN] = {
	MAGIC_HEX_0, MAGIC_HEX_1,
	MAGIC_HEX_2, MAGIC_HEX_3
};

#define IOT_UART_BANDRATE 115200
#define IOT_UART_DATABITS 8
#define IOT_UART_STOPBITS 1
#define IOT_UART_PARITY   'N'

static inline int byte_match(uint8_t c)
{
	uint8_t ch = 0;

	int ret = uart_read_nonblock(&ch, 1 , 1000);
	printf("%d , %x\n" , ret , ch);
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

static void check_hitinga_alive()
{
    //此处添加查询hitinga模组是否还活着
    return;
}

static void UartReadProc(void *params)
{
	uint8_t head[12] = {0};
	uint8_t *streambyte = NULL;
	int streamlen = 0;
	uint16_t bodylen = 0;

	// 这里为了握手设备
    uint64_t old = 0 , now = 0;

	old = get_time_s();

	for(;;){
		do{
			// 如果没有查询到设备ID，则一直查询
			if(1){
            	now = get_time_s();
				if(now - old > 5){
                	old = get_time_s();
                    check_hitinga_alive();
				}
			}
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
	uart_init();

    if(uart_config(IOT_UART_BANDRATE , IOT_UART_DATABITS , IOT_UART_STOPBITS , IOT_UART_PARITY) != 0){
        return -1;
    }
    else{
        DEBUG_PRINTF("uart init success!!!\n");
    }

    if(create_thread((void *)UartReadProc , NULL) != 0){
        return -1;
    }
    else{
        DEBUG_PRINTF("create thread success!!!\n");
    }

    for(;;){
		debugline;
        msleep(1000);
    }

    return 0;

}
