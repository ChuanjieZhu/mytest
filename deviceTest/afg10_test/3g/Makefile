
TARGET = 3g_test

CC = arm-linux-gcc
CXX = arm-linux-g++
LD = arm-linux-ld
AR = arm-linux-ar

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp = .o)

INCLUDE = -I./curl
LIBS = curl/libcurl.a
LIBS += -lpthread
LIBS += -lrt

CCFLAGS = -g -Wall -O2

$(TARGET) : $(OBJS)	
	$(CXX) $^ -o $@ $(INCLUDES) $(LIBS)

#%.o : %.c	
#	$(CC) -c $< $(CCFLAGS)

%.o : %.cpp
	$(CXX) -c $< $(CCFLAGS)

clean:	
	rm *.o $(TARGET)

.PHONY:clean

