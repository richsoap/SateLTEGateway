cxx = g++
tail = -lpjsua2-x86_64-unknown-linux-gnu -lstdc++ -lpjsua-x86_64-unknown-linux-gnu -lpjsip-ua-x86_64-unknown-linux-gnu -lpjsip-simple-x86_64-unknown-linux-gnu -lpjsip-x86_64-unknown-linux-gnu -lpjmedia-codec-x86_64-unknown-linux-gnu -lpjmedia-x86_64-unknown-linux-gnu -lpjmedia-videodev-x86_64-unknown-linux-gnu -lpjmedia-audiodev-x86_64-unknown-linux-gnu -lpjmedia-x86_64-unknown-linux-gnu -lpjnath-x86_64-unknown-linux-gnu -lpjlib-util-x86_64-unknown-linux-gnu  -lsrtp-x86_64-unknown-linux-gnu -lresample-x86_64-unknown-linux-gnu -lgsmcodec-x86_64-unknown-linux-gnu -lspeex-x86_64-unknown-linux-gnu -lilbccodec-x86_64-unknown-linux-gnu -lg7221codec-x86_64-unknown-linux-gnu  -lpj-x86_64-unknown-linux-gnu -lssl -lcrypto -lm -lrt -lpthread   
files = socket_rece.o socket_send.o socketmedia.o socket_thread.o

main: $(files) main_101.o main_102.o
	$(cxx) -o demo_101 main_101.o $(files) $(tail)
	$(cxx) -o demo_102 main_102.o $(files) $(tail)

clean:
	rm *.o
