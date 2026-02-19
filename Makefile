CC=zig c++
OPTIONS=-std=c++17 -O2

all: meta24

meta24: meta24.cc
	$(CC) $(OPTIONS) -DMP11 -I mp11/include -o meta24_zig meta24.cc

clean:
	rm -f meta24
