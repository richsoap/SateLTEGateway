cxx = g++
exe = demo
tail = -lpthread -ljrtp -std=c++11 -I/usr/local/include/jrtplib3
objs = main.o
file = main.cpp

all: 
	$(cxx) -c $(file) $(tail)
	$(cxx) -o $(exe) $(objs) $(tail)

clean:
	rm *.o
