#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <termios.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include "porting.h"

static int uart_fd;
static void * mutex_uartsend = NULL;

void *OS_MALLOC(size_t size)
{
    if(size <= 0){
        ERROR_PRINTF("error input malloc size\n");
        return NULL;
    }

    return malloc(size);
}

void OS_FREE(void *p)
{
    if(p == NULL){
        ERROR_PRINTF("error want free pointer\n");
        return;
    }
    free(p);
}

void *OS_MEMCPY(void *dest, const void *src, size_t n)
{
    return memcpy(dest , src , n);
}


void *OS_MEMSET(void *s, int c, size_t n)
{
    return memset(s , c , n);
}


static int OpenSerial(const char *dev) {
	struct termios opt;
	int fd = open(dev, O_RDWR);

	if (tcgetattr(fd, &opt) == -1) {
		close(fd);
		fd = -1;
	}

	return fd;
}

static int Open(const char* dev) {
	int fd;

	if (dev != NULL) {
		fd = OpenSerial(dev);
		if(fd < 0){
			printf("open serial %s failed, please check.\n", dev);
			return -1;
		}
	}

	return fd;
}

static int Close(int fd) {
	if (fd > 0)
		close(fd);

	return 0;
}

static int Config(int fd, int speed, int databits, int stopBits, char parity) 
{
	int spd;
	struct termios opt;

	/* waits until all output written to the object referred to by fd has been transmitted */
	tcdrain(fd);

	/* flushes both data received but not read, and data written but not transmitted. */
	tcflush(fd, TCIOFLUSH);

	if (tcgetattr(fd, &opt) != 0) {
		ERROR_PRINTF("Serial config tcgetattr error.\n");
		return -1;
	}

	switch (speed / 100) {
    	case 0:
#ifdef B0
    		spd = B0;
#else
    		spd = 0;
#endif
    		break;
    	case 3: spd = B300; break;
    	case 6: spd = B600; break;
    	case 12:    spd = B1200;    break;
    	case 24:    spd = B2400;    break;
    	case 48:    spd = B4800;    break;
    	case 96:    spd = B9600;    break;
#ifdef B19200
    	case 192:   spd = B19200;   break;
#else /* B19200 */
#ifdef EXTA
    	case 192:   spd = EXTA; break;
#else /* EXTA */
    	case 192:   spd = B9600;    break;
#endif /* EXTA */
#endif   /* B19200 */
#ifdef B38400
    	case 384:   spd = B38400;   break;
#else /* B38400 */
#ifdef EXTB
    	case 384:   spd = EXTB; break;
#else /* EXTB */
    	case 384:   spd = B9600;    break;
#endif /* EXTB */
#endif   /* B38400 */
#ifdef B57600
    	case 576:   spd = B57600;   break;
#endif
#ifdef B115200
    	case 1152:  spd = B115200;  break;
#endif
#ifdef B230400
    	case 2304:  spd = B230400;  break;
#endif
#ifdef B460800
    	case 4608: spd = B460800; break;
#endif
#ifdef B500000
    	case 5000: spd = B500000; break;
#endif
#ifdef B576000
    	case 5760: spd = B576000; break;
#endif
#ifdef B921600
    	case 9216: spd = B921600; break;
#endif
#ifdef B1000000
    	case 10000: spd = B1000000; break;
#endif
#ifdef B1152000
    	case 11520: spd = B1152000; break;
#endif
#ifdef B1500000
    	case 15000: spd = B1500000; break;
#endif
#ifdef B2000000
    	case 20000: spd = B2000000; break;
#endif
#ifdef B2500000
    	case 25000: spd = B2500000; break;
#endif
#ifdef B3000000
    	case 30000: spd = B3000000; break;
#endif
#ifdef B3500000
    	case 35000: spd = B3500000; break;
#endif
#ifdef B4000000
    	case 40000: spd = B4000000; break;
#endif
    	default:
    		ERROR_PRINTF("unsupport baudrate : %d\n", speed);
    		return -1;
	}

	if (cfsetispeed(&opt, spd) != 0)
		ERROR_PRINTF("Serial config cfsetispeed error\n");
	if (cfsetospeed(&opt, spd) != 0)
		ERROR_PRINTF("Serial config cfsetospeed error\n");

	opt.c_cflag &= ~CSIZE;
	switch (databits) { /*设置数据位数*/
	case 7:
		opt.c_cflag |= CS7;
		break;
	case 8:
		opt.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr,"Unsupported data size\n");
		return -1;
	}
	switch (parity) {
	case 'n':
	case 'N':
		opt.c_cflag &= ~PARENB;           /* Clear parity enable */
		opt.c_iflag &= ~INPCK;            /* Enable parity checking */
		break;
	case 'o':
	case 'O':
		opt.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
		opt.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'e':
	case 'E':
		opt.c_cflag |= PARENB;            /* Enable parity */
		opt.c_cflag &= ~PARODD;           /* 转换为偶效验*/
		opt.c_iflag |= INPCK;             /* Disnable parity checking */
		break;
	case 'S':
	case 's':  /*as no parity*/
		opt.c_cflag &= ~PARENB;
		opt.c_cflag &= ~CSTOPB;break;
	default:
		fprintf(stderr,"Unsupported parity\n");
		return -1;
	}

	/* 设置停止位*/
	switch (stopBits) {
	case 1:
		opt.c_cflag &= ~CSTOPB;
		break;
	case 2:
		opt.c_cflag |= CSTOPB;
		break;
	default:
		fprintf(stderr,"Unsupported stop bits\n");
		return -1;
	}
	/* Set input parity option */
	if (parity != 'n')
		opt.c_iflag |= INPCK;

	opt.c_cc[VTIME] = 150; /* 设置超时 15 seconds          */
	opt.c_cc[VMIN] = 0;    /* Update the opt and do it NOW */

	opt.c_cflag &= ~CRTSCTS;
	opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);  /* Input  */
	opt.c_oflag &= ~OPOST;                           /* Output */
	opt.c_iflag &= ~(INLCR | ICRNL | IGNCR | IXON);

	if (tcsetattr(fd, TCSANOW, &opt) != 0) {
		fprintf(stderr, "Serial config tcsetattr error.\n");
		return -1;
	}

	return 0;
}

/*readTimeout、writeTimeout单位为秒*/
static int setTimeouts(int fd, int readTimeout, int writeTimeout) 
{
	struct termios opt;
	readTimeout=readTimeout*10;
	writeTimeout=writeTimeout*10;
	if (tcgetattr(fd, &opt) == -1)
	{
		return -1;
	}
	opt.c_cc[VTIME] = readTimeout; /* 设置超时*/
	opt.c_cc[VMIN] = 0;    /* Update the opt and do it NOW */
	if (tcsetattr(fd, TCSANOW, &opt) != 0)
	{
		return -1;
	}
	return 0;
}

int TryRead(int fd, void *buf, int size) 
{
	struct termios opt;

	if (tcgetattr(fd, &opt) == -1) {
		return -1;
	}

	// 不阻塞
	opt.c_cc[VTIME] = 0;
	opt.c_cc[VMIN]  = 0;
	if (tcsetattr(fd, TCSANOW, &opt) != 0) {
		return -1;
	}
	return read(fd, buf, size);
}

int Read(int fd, void *buf, int size) {
	return read(fd, buf, size);
}

int BlockRead(int fd, void *buf, int size) 
{
	int len = 0;
	int act_len = 0;
	while(size) {
		len = read(fd, (void *)((char*)buf + act_len), size);
		size -= len;
		act_len += len;
	}
	return act_len;
}

static int Write(int fd, const void *buf, int size) 
{
	return write(fd, buf, size);
}

/*
 * uart_init : uart 初始化
 *  return : 配置成功返回 0，配置失败返回负值
 */
int uart_init(void)
{
	uart_fd = Open("/dev/ttyUSB0");
	if (uart_fd < 0) {
		printf ("uart open failed\n");
		return -1;
	}

	return 0;
}

/*
 * uart_done : uart 资源释放
 *  return : 配置成功返回 0，配置失败返回负值
 */
int uart_done(void)
{
	Close(uart_fd);
	return 0;
}

/*
 * uart_config :
 *  @baudrate : 波特率
 *  @databits : 数据位
 *  @stopbits : 停止位
 *  @parity : 奇偶校验
 *  return : 配置成功返回 0，配置失败返回负值
 */
int uart_config(int baudrate, int databits, int stopbits, char parity)
{
	return Config(uart_fd, baudrate, databits, stopbits, parity);
}

/*
 * uart_read : 阻塞接收串口数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int uart_read(uint8_t *buf, int len)
{
	return BlockRead(uart_fd, buf, len);
}

/*
 * uart_try_read : 尝试从串口中读取数据（立即返回）
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int uart_try_read(uint8_t *buf, int len)
{
	return TryRead(uart_fd, buf, len);
}

/*
 * uart_read_nonblock : 超时从串口中读取数据
 *  @buf : 串口数据接收缓冲
 *  @len : 期望接收的数据长度
 *  @timeout_ms : 超时时间设置
 *  return : 返回串口实际收到的数据长度，出错则返回负值
 */
int uart_read_nonblock(uint8_t *buf, int len, int timeout_ms)
{
	setTimeouts(uart_fd, timeout_ms, timeout_ms);
	return Read(uart_fd, buf, len);
}

/*
 * uart_write : 串口写数据
 *  @buf : 串口数据发送缓冲
 *  @len : 待发送数据的长度
 *  return : 返回串口实际发送的数据长度，出错则返回负值
 */
int uart_write(const uint8_t *buf, int len)
{
    LOCK(mutex_uartsend);
    int ret = Write(uart_fd, buf, len);
    UNLOCK(mutex_uartsend);
	return ret;
}


/*
 * msleep : 睡眠
 */
void msleep(int ms)
{
	usleep(ms * 1000);
	return ;
}

/*
 * get_time_ms : 获取当前系统时间
 */
uint64_t get_time_ms(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

/*
 * get_time_s : 获取当前系统时间
 */
uint64_t get_time_s(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec);
}

void *mutex_create()
{
    pthread_mutex_t * mutex = (pthread_mutex_t *)OS_MALLOC(sizeof(pthread_mutex_t));
    pthread_mutexattr_t attr;
    if (pthread_mutexattr_init(&attr) == 0)
    {
        if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0)
            pthread_mutex_init(mutex, &attr);
        pthread_mutexattr_destroy(&attr);
    }
    return mutex;
}

int create_thread(void * thread_function, void * user_object)
{
    int res = 0;

    mutex_uartsend = mutex_create();


    pthread_t thread;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == 0)
    {
        if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) == 0)
        {
            if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0)
            {
                res = pthread_create(&thread, &attr, (void *)thread_function, user_object);
            }
        }
        pthread_attr_destroy(&attr);
    }

    return res;
}

void LOCK(void *mutex)
{
	if (mutex) pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void UNLOCK(void *mutex)
{
	if (mutex) pthread_mutex_unlock((pthread_mutex_t *)mutex);
}
