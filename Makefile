#-*- mode:makefile-gmake; -*-
ROOT = $(shell pwd)
TARGET = ai4g

INCLUDE += -I$(ROOT)/src
SOURCES = $(wildcard $(ROOT)/src/*.c)

pkg-config = $(shell pkg-config --$(1) --static $(2))

OBJS = $(patsubst %.c,%.o,$(SOURCES))
CPPFLAGS = $(OPTCPPFLAGS)
SDL_LIBS = $(call pkg-config,libs,SDL2_image) $(call pkg-configs,libs,sdl2)
SDL_CFLAGS = $(call pkg-config,cflags,SDL2_image) $(call pkg-configs,cflags,sdl2)
LIBS = $(SDL_LIBS) $(OPTLIBS)
CFLAGS = -g -std=c99 $(INCLUDE) -Wall -Werror $(OPTFLAGS) $(SDL_CFLAGS)

.DEFAULT_GOAL = all
all: $(TARGET)

clean:
	rm -rf $(OBJS) $(TARGET) $(TARGET).o $(TARGET).new

.PHONY: all clean test

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ $(LIBS) -o $@.new
	mv $@.new $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $^
