TARGET = config

ifdef s3c2440
	export CC = arm-linux-g++
	export AR = arm-linux-ar
else ifdef TI6446
	export CC = arm_v5t_le-g++ 
	export AR = arm_v5t_le-ar 
else
	export CC = g++
	export AR = ar
endif

ifdef S3C2440 
	FLAGS += -DPLAT_S3C2440
else ifdef TI6446 
	FLAGS += -DPLAT_TI6446
else

endif

FLAGS += -Wall -O3 -lm

OBJS += sysConfig.o myXml/myXml.o myXml/tinyxmlerror.o myXml/tinyxml.o 
OBJS += myXml/tinyxmlparser.o myXml/tinystr.o

%.o: %.cxx $(COMPILER_FILE)
	$(CC) -c $(FLAGS) -o $@ $<

%.o: %.cpp $(COMPILER_FILE)
	$(CC) -c $(FLAGS) -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $^
	
clean:
	rm -f *.o $(TARGET)
	