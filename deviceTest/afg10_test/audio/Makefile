
#====================================================#
#  Makefile																	
#====================================================#

TARGET = audio_test

CC = arm-linux-gcc
CXX = arm-linux-g++
LD = arm-linux-ld
AR = arm-linux-ar

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
LIBS = ./2451/AiSound5.a
CPPFLAGS = -g -Wall -lpthread

ifdef DEBUG_MEMORY
	CPPFLAGS += -DDEBUG_MEMORY
endif


$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@ $(CPPFLAGS) $(LIBS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean

	
