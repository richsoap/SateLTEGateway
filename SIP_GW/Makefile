cxx = g++
file = ./lib/libosipparser2.a ./lib/libAMR.a ./lib/libgsm.a ./lib/libstopwatch.a
tail = -I./include -std=c++11 -lpthread -DGSM -lboost_system -lboost_program_options


all: 
	$(cxx) sip_gw.cpp $(file) $(tail) -o sip
	$(cxx) rtp_gw.cpp $(file) $(tail) -o rtp

