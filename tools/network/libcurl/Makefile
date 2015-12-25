
#====================================================#
#  Makefile																	
#====================================================#

TARGET = libcurl_test

CC = arm_v5t_le-gcc
CXX = arm_v5t_le-g++

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

OBJS = $(CSRCS:.c = .o)
OBJS += $(CPPSRCS:.cpp = .o)

INCLUDE = -I/xxx
CPPFLAGS = -g -Wall -lpthread

ifdef DEBUG_MEMORY
	CPPFLAGS += -DDEBUG_MEMORY
endif

LIBS += libcurl.a
LIBS += libjson.a
LIBS += -lrt


$(TARGET) : $(OBJS)
	$(CXX) $^ -o $@ $(CPPFLAGS) -L. $(LIBS)
	cp $(TARGET) /nfsroot/dm644x/filesys/opt/dm644x

%.o: %.c 
	$(CC) -c $(CPPFLAGS) -o $@ $< 

%.o: %.cxx
	$(CXX) -c $(CPPFLAGS) -o $@ $<

clean:
	rm *.o $(TARGET)

.PHONY:clean
