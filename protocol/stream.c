#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "iot_stream.h"
#include "crc32.h"
#include "porting.h"

/*
 * 将字节流结构变成json协议数据
 */
/*
 * 创建一个字节流的结构
 */
stream_msg_t * HTIOTStreamCreate()
{
	stream_msg_t * stream = OS_MALLOC(sizeof(stream_msg_t));
	if(stream==NULL){
		ERROR_PRINTF("out-of-memory\n");
		return NULL;
	}

	OS_MEMSET(stream,0x0,sizeof(stream_msg_t));

	stream->magic = MAGIC_HEX;
	stream->equipment = IOT_EQUIP_ID;
    stream->vendor = IOT_VENDOR_ID;
    stream->version = IOT_VERSION_ID;

	return stream;
}

/*
 * 为字节流结构增加一个参数
 * 参数存放在body中
 */
int HTIOTStreamAddNode(stream_msg_t * stream, uint32_t pid , uint32_t value)
{
	int paramlen = sizeof(stream_param_t);

	// 重新分配刚好大小的连续空间
	uint8_t * body = OS_MALLOC(stream->bodylen+paramlen);
	if(body==NULL){
		ERROR_PRINTF("out-of-memory\n");
		return -1;
	}

	// 如果存在，删除旧的body
	if(stream->body){
		OS_MEMCPY(body , stream->body , stream->bodylen);
		OS_FREE(stream->body);
	}

	// 获取新的指针
	stream->body = body;

	// 赋值
	stream_param_t * param = (stream_param_t *)(stream->body + stream->bodylen);
	param->pid=(uint32_t)pid;
	param->value=(uint32_t)value;

	// 赋值bodylen的长度
	stream->bodylen += paramlen;
	
	return 0;
}

/*
 * 将字节流结构，变成一个字节流
 */
int HTIOTStreamPrintBuf(stream_msg_t * stream,unsigned char ** ppbuffer , unsigned short * plen)
{
	int totallen = sizeof(stream_msg_t) - sizeof(uint8_t *) + stream->bodylen;

	unsigned char *pbuffer = OS_MALLOC(totallen);
	if(pbuffer==NULL){
		ERROR_PRINTF("out-of-memory\n");
		return -1;
	}

	stream_msg_t * pst = (stream_msg_t*)pbuffer;

	// 赋值头部结构
	pst->magic = stream->magic;
	pst->version = stream->version;
	pst->equipment = stream->equipment;
	pst->vendor = stream->vendor;
	pst->bodylen = stream->bodylen;

	// 把数据copy到头的后面
	OS_MEMCPY(&(pst->body), stream->body , stream->bodylen);

	// 计算所有内容的32
	int icrc32 = crc32((unsigned char *)pst , totallen-4);
	OS_MEMCPY(&pbuffer[totallen-4],(void*)&icrc32,4);

	*ppbuffer = pbuffer;
	*plen = totallen;

	return 0;
}

/*
 * 将一个字节流，变成结构
 */
int HTIOTParseToStream(stream_msg_t ** ppstream,unsigned char * pbuffer , unsigned short len)
{
	// 校验323
	uint32_t old_crc32=*(unsigned int *) (&pbuffer[len-4]);
	uint32_t new_crc32 = crc32((unsigned char *)pbuffer , len-4);
	if(old_crc32!=new_crc32){
		*ppstream=NULL;
		DEBUG_PRINTF("old_crc32[0x%08x],new_crc32[0x%08x]\n" , old_crc32 , new_crc32);
		return -1;
	}

	// 创建一个stream
	stream_msg_t * pstream = OS_MALLOC(sizeof(stream_msg_t));
	if(pstream==NULL){
		ERROR_PRINTF("out-of-memory\n");
		return -1;
	}

	OS_MEMCPY((void*)pstream, pbuffer, sizeof(stream_msg_t));

	pstream->body = OS_MALLOC(pstream->bodylen);
	if(pstream->body==NULL){
		ERROR_PRINTF("out-of-memory\n");
		OS_FREE(pstream);
		return -1;
	}

	OS_MEMCPY(pstream->body ,&pbuffer[12],pstream->bodylen);
	pstream->crc32 = new_crc32;

	*ppstream = pstream;

	return 0;
}

/*
 * 销毁一个字节流结构
 */
void HTIOTStreamDelete(stream_msg_t * stream)
{
	if(stream){
		if(stream->body){
			OS_FREE(stream->body);
		}

		OS_FREE(stream);
	}

	return;
}


