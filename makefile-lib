CC = g++
CFLAGS  = -fPIC -g 
LDFLAGS = -shared

TARGET  = libFAT32.so

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): 
	$(CC) $(CFLAGS) $(LDFLAGS) myFat32.cc -o $(TARGET)
