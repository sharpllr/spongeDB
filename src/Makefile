CC := gcc
libs := -lpthread

sources := $(wildcard include/util/*.c ds/*.c src/*.c *.c)
objects := $(patsubst  %.c, %.o, $(sources))

all:
	$(MAKE) -C $(objects) $(libs)