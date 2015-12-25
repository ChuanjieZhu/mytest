#====================================================#
#  Makefile																	
#====================================================#

TARGET = gps_test

CC = arm-linux-gcc
CXX = arm-linux-g++
LD = arm-linux-ld
AR = arm-linux-ar

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
LIBS = 
CPPFLAGS = -g -Wall -lpthread -s

ifdef DEBUG_MEMORY
	CPPFLAGS += -DDEBUG_MEMORY
endif


$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@ $(CPPFLAGS) $(LIBS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cpp
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean

