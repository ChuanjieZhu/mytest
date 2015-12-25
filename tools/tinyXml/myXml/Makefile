ifdef X86
CC = g++
else ifdef S3C2440
CC = arm-linux-g++
AR = arm-linux-ar
else
CC = arm_v5t_le-gcc
endif

all:
	$(CC) -c -Wall -Wno-unknown-pragmas -Wno-format -O3   tinyxml.cpp -o tinyxml.o
	$(CC) -c -Wall -Wno-unknown-pragmas -Wno-format -O3   tinyxmlparser.cpp -o tinyxmlparser.o
	$(CC) -c -Wall -Wno-unknown-pragmas -Wno-format -O3   tinyxmlerror.cpp -o tinyxmlerror.o
	$(CC) -c -Wall -Wno-unknown-pragmas -Wno-format -O3   tinystr.cpp -o tinystr.o
	$(CC) -c -Wall -Wno-unknown-pragmas -Wno-format -O3   myXml.cpp -o myXml.o
	$(AR) -r libmyXml.a tinyxml.o tinyxmlparser.o tinyxmlerror.o tinystr.o myXml.o

clean:
	rm *.o -f

