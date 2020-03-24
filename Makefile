CC = gcc

#librws================================================================
#CFLAGS := -Wformat=0 -Wno-error=unused-value
LDFLAGS = -lpthread

INCLUDEDIRS = -I./include -I./porting -I./protocol

SRCS += main.c

SRCS += $(wildcard ./porting/*.c)
SRCS += $(wildcard ./protocol/*.c)


OBJS = $(SRCS:.c=.o)

CFLAGS += -Wall -g

%.o:%.c
	$(CC) -c $< -o $@ $(CFLAGS) $(INCLUDEDIRS)

all:$(OBJS)
	$(CC) $^ -o lightdemo $(LDFLAGS)

clean:
	-rm -f $(OBJS) lightdemo

