#====================================================#
#  Makefile																	
#====================================================#

TARGET = cjsonTest

CC = gcc
CXX = g++
LD = ld
AR = ar

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
LIBS = -L/xxx
CPPFLAGS = -g -Wall -lpthread -lm

$(TARGET) : $(OBJS)
	$(CC) $^ -o $@ $(CPPFLAGS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

clean:
	rm *.o $(TARGET)

.PHONY:clean
