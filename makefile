default:  
	g++ myFat32.cc -o fat
lib:  
	g++ -shared -o libFAT32.so -fPIC myFat32.cc
clean:
	rm fat
