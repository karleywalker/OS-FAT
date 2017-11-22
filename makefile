CC = g++
CFLAGS  = -fPIC -g -std=c++11
LDFLAGS = -shared

TARGET  = libFAT32.so

all: $(clean) $(TARGET)

clean:
	rm -rf $(TARGET) myFat32.o testCd testOpen testClose testDirRead testFileRead tok

$(TARGET): 
	$(CC) $(CFLAGS) -c myFat32.cc -o myFat32.o
	$(CC) $(LDFLAGS) myFat32.o -o $(TARGET)

test:
	$(CC) -std=c++11 test_open.cc -o testOpen $(TARGET)
	$(CC) -std=c++11 test_readDir.cc -o testDirRead $(TARGET)
	$(CC) -std=c++11 test_cd.cc -o testCd $(TARGET)
	$(CC) -std=c++11 test_close.cc -o testClose $(TARGET)
	$(CC) -std=c++11 test_read.cc -o testFileRead $(TARGET)
