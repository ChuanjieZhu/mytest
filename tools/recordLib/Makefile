#====================================================#
#  Makefile																	
#====================================================#

TARGET = logLibTest

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
CPPFLAGS = -g -Wall -lpthread

ifdef DEBUG_MEMORY
	CPPFLAGS += -DDEBUG_MEMORY
endif


$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@ $(CPPFLAGS)

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean

