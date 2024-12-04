CC=zig c++
OPTIONS=-std=c++17 -O2 -ftemplate-depth=4000 -s -static

all: meta24_mp11 meta24_metal

meta24_mp11: meta24.cc
	$(CC) $(OPTIONS) -DMP11 -I mp11/include -o meta24_mp11 meta24.cc

meta24_metal: meta24.cc
	$(CC) $(OPTIONS) -DMETAL -I metal/include -o meta24_metal meta24.cc

clean:
	rm -f meta24_mp11 meta24_metal
