
TARGET = dhtest

ifdef ARM
CC = arm_v5t_le-gcc
CXX = arm_v5t_le-g++
LD = arm_v5t_le-ld
AR = arm_v5t_le-ar
else
CC = gcc
CXX = g++
LD = ld
AR = ar
endif

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp = .o)

INCLUDE = -I
LIBS += -lpthread
LIBS += -lrt

CCFLAGS += -Wall -O3 -lm -Wcast-align -Wpadded -Wpacked

$(TARGET) : $(OBJS)	
	$(CXX) $^ -o $@ $(CCFLAGS) $(INCLUDES) $(LIBS)

%.o : %.c	
	$(CC) -c $< $(CCFLAGS)

%.o : %.cpp
	$(CXX) -c $< $(CCFLAGS)

clean:	
	rm -f $(TARGET)

.PHONY:clean

