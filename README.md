# HT5X
HT5100X/HT5200X

Submit a demonstration program for serial communication of HT5X. This is a standard C code and the running environment is ubuntu linux.

The demo sample compilation only requires make.

The serial port read in the code uses a TTY device. When running, please confirm that the TTY serial device of ubuntu can work normally.

If you need to port to other embedded devices, you can always use uart_init / uart_read / uart_read_nonblock / uart_write / uart_config in porting.c.

For specific hardware environment of HT5X, please send an email to developer@hitinga.com, or follow us on WeChat (嗨豆丁智能) public account
