cxx = g++
file = /usr/local/lib/libosipparser2.a
tail = -I./include -std=c++11 -lpthread


all: 
	$(cxx) sip_gw.cpp $(file) $(tail) -o sip
	$(cxx) rtp_gw.cpp $(file) $(tail) -o rtp

