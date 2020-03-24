#ifndef PORTING_H
#define PORTING_H

#include <stdint.h>

#define DEBUG_PRINTF(fmt,args...) \
do {\
	    printf( "DEBUG[%s:%d]:" fmt, __FUNCTION__,__LINE__,##args );\
    }while (0)

#define ERROR_PRINTF(fmt,args...) \
do {\
	    printf( "ERROR[%s:%d]:" fmt, __FUNCTION__,__LINE__,##args );\
    }while (0)

#define debugline   do{\
        printf("%s:%d\n" ,__FUNCTION__ , __LINE__ );\
    }while(0)

void *OS_MALLOC(size_t size);
void OS_FREE(void *p);
void *OS_MEMCPY(void *dest, const void *src, size_t n);
void *OS_MEMSET(void *s, int c, size_t n);

/* The following API need to be ported */

/*
 * uart_init : uart 初始化
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t uart_init(void);

/*
 * uart_done : uart 资源释放
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t uart_done(void);

/*
 * uart_config : 配置 uart 的波特率等信息
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
int32_t uart_config(int32_t baudrate, int32_t databits, int32_t stopbits, char parity);

/*
 * uart_read : 阻塞接收串口数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t uart_read(uint8_t *buf, int32_t len);

/*
 * uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t uart_try_read(uint8_t *buf, int32_t len);

/*
 * uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int32_t uart_read_nonblock(uint8_t *buf, int32_t len, int32_t timeout_ms);

/*
 * uart_write : 串口写数据
 *  @buf : 串口数据发送缓冲
 *  @len : 待发送数据的长度
 *  return : 返回串口实际发送的数据长度，出错则返回负值
 */
int32_t uart_write(const uint8_t *buf, int32_t len);

/*
 * msleep : 睡眠
 */
void msleep(int32_t ms);

/*
 * get_time_ms : 获取当前系统时间
 */
uint64_t get_time_ms(void);

uint64_t get_time_s(void);


void *mutex_create();

int create_thread(void * thread_function, void * user_object);

void LOCK(void *mutex);
void UNLOCK(void *mutex);

#endif
