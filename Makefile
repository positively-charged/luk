# Nasty-looking GNU Makefile for building luk.

PROG_NAME = luk

DIR_SRC = src
DIR_LIB = lib
DIR_CONF = $(DIR_LIB)/conf
DIR_MD5 = $(DIR_LIB)/md5
DIR_HUFF = $(DIR_LIB)/huffman
DIR_OUT = out

# We can't use the C compiler now because the Huffman implementation
# is now in C++.
#
# CC = gcc
# CFLAGS = -std=c89 -ansi -pedantic -Wall -Werror \
#    -Wextra -Wunused -Wno-unused-function -pedantic-errors \
#    -Wmissing-prototypes

CC = g++
INCLUDE = -I lib -I lib/conf -I lib/huffman -I lib/md5
COMPILE = $(CC) $(CFLAGS) $(INCLUDE) -c

OUT_SRC := $(patsubst $(DIR_SRC)/%.c,$(DIR_OUT)/%.o,\
   $(wildcard $(DIR_SRC)/*.c))
OUT_LIB := $(patsubst $(DIR_LIB)/%.c,$(DIR_OUT)/%.o,\
   $(wildcard $(DIR_LIB)/*.c))
OUT_CONF := $(patsubst $(DIR_CONF)/%.c,$(DIR_OUT)/%.o,\
   $(wildcard $(DIR_CONF)/*.c))
OUT_MD5 := $(patsubst $(DIR_MD5)/%.c,$(DIR_OUT)/%.o,\
   $(wildcard $(DIR_MD5)/*.c))
OUT_HUFF := $(patsubst $(DIR_HUFF)/%.cpp,$(DIR_OUT)/%.o,\
   $(wildcard $(DIR_HUFF)/*.cpp))

# Program:
# --------------------------------------------------------

$(PROG_NAME): $(DIR_OUT) $(OUT_SRC) $(OUT_LIB) $(OUT_MD5) \
   $(OUT_CONF) $(OUT_HUFF)
	$(CC) $(CFLAGS) -o $(PROG_NAME) $(DIR_OUT)/*.o

$(DIR_OUT): 
	@if [ ! -d $(DIR_OUT) ]; then \
	  mkdir $(DIR_OUT); \
	fi

$(DIR_OUT)/%.o: $(DIR_SRC)/%.c 
	$(COMPILE) -o $@ $<

clean:
	rm $(PROG_NAME)
	rm $(DIR_OUT)/*.o
	rmdir $(DIR_OUT)

# Libraries:
# --------------------------------------------------------

$(DIR_OUT)/%.o: $(DIR_LIB)/%.c 
	$(COMPILE) -o $@ $<

$(DIR_OUT)/%.o: $(DIR_CONF)/%.c 
	$(COMPILE) -o $@ $<

$(DIR_OUT)/%.o: $(DIR_MD5)/%.c 
	$(COMPILE) -o $@ $<

$(DIR_OUT)/%.o: $(DIR_HUFF)/%.cpp 
	$(COMPILE) -o $@ $<

