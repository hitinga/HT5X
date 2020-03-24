#ifndef AIOT_STREAM_H
#define AIOT_STREAM_H

#define MAGIC_HEX  0x47474747
#define MAGIC_HEX_0 (((MAGIC_HEX) & 0x000000FF) >> 0 )
#define MAGIC_HEX_1 (((MAGIC_HEX) & 0x0000FF00) >> 8 )
#define MAGIC_HEX_2 (((MAGIC_HEX) & 0x00FF0000) >> 16)
#define MAGIC_HEX_3 (((MAGIC_HEX) & 0xFF000000) >> 24)

#define MAGIC_LEN      4     /* Message magic length */
#define STREAM_CRC_LEN 4

#define IOT_VERSION_ID 0x01
#define IOT_EQUIP_ID 0x02
#define IOT_VENDOR_ID 0x01

/* 整体协议定义  */
typedef struct {
	uint32_t magic;
	uint16_t version;
	uint16_t equipment;
	uint16_t vendor;
	uint16_t bodylen;
	uint8_t *body;

	//实际的传输流中，最后body后面，连接这一个整形
	//body和bodylen是连续的数据字节流
	uint32_t crc32;
}stream_msg_t;

/* 字节协议的定义 */
typedef struct {
	uint32_t pid;
	uint32_t value;
}stream_param_t;

enum{
	IOT_PAR_MODE_AUTO,
	IOT_PAR_MODE_MANUAL,
	IOT_PAR_MODE_SUCCESS,
	IOT_PAR_MODE_FAILED,

	IOT_PAR_MODE_END
};

/* 其中body 的规约是连续的结构体字节序 */
/*
 * {
 *	|id1     |, | value        |
 *	|id2     |, | value        |
 *	|id3     |, | value        |
 *	|id4     |, | value        |
 *
 *	sum((4+4)+(4+4)+(4+4)+(4+4)) = bodylen
 * }
 *
 */
extern stream_msg_t * HTIOTStreamCreate();

extern int HTIOTStreamAddNode(stream_msg_t * stream, uint32_t pid , uint32_t value);

extern int HTIOTStreamPrintBuf(stream_msg_t * smsg,unsigned char ** ppbuffer , unsigned short * plen);

extern int HTIOTParseToStream(stream_msg_t ** ppstream,unsigned char * pbuffer , unsigned short len);

extern void HTIOTStreamDelete(stream_msg_t * smsg);

extern void dumphex(unsigned char * data, int len );


#endif

