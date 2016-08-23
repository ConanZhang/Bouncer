# Authors: Charles Khong & Conan Zhang
# Date: March 9, 2015
#
# Description: Make for bouncer application.

all: bouncer

bouncer: bouncer.cpp bouncer.h
	g++ bouncer.cpp -o bouncer `pkg-config --cflags --libs libavutil libavformat libavcodec libswscale`
clean:
	rm -f bouncer *.o *.mp4 *.mpff

movie:
	ffmpeg -framerate 30 -start_number 000 -i frame%03d.mpff movie.mp4
