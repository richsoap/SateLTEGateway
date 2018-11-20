cxx = g++
exe = demo
tail = -lpthread -std=c++11
objs = main.o
file = main.cpp

all: 
	$(cxx) -c $(file) $(tail)
	$(cxx) -o $(exe) $(objs) $(tail)

clean:
	rm *.o
