cxx = g++
exe = gsm_demo
tail = -lpthread -std=c++11
objs = gsm_main.o
file = gsm_main.cpp


all: 
	$(cxx) -c $(file) $(tail)
	$(cxx) -o $(exe) $(objs) $(tail)

clean:
	rm *.o
