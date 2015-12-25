
#====================================================#
#  Makefile																	
#====================================================#

TARGET = 2451Test

CC = arm-linux-gcc
CXX = arm-linux-g++
LD = arm-linux-ld
AR = arm-linux-ar

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
LIBS = ejTTS.a
CPPFLAGS = -g -Wall -lpthread

ifdef DEBUG_MEMORY
	CPPFLAGS += -DDEBUG_MEMORY
endif



$(TARGET) : $(OBJS)
	arm-linux-gcc -c Win_Linux_file_func.c -o Win_Linux_file_func.o
	$(CXX) $^ -o $@ $(CPPFLAGS) Win_Linux_file_func.o -L. -lCvsCore $(LIBS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean
