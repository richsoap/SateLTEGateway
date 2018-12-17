cxx = g++
file = ./lib/libosipparser2.a ./lib/libAMR.a ./lib/libgsm.a ./lib/libstopwatch.a
tail = -I./include -std=c++11 -lpthread -DGSM


all: 
	$(cxx) sip_gw.cpp $(file) $(tail) -o sip
	$(cxx) rtp_gw.cpp $(file) $(tail) -o rtp

