###########################################################################
#
# MODULE:             Iotc Net - Makefile
#
# REVISION:           $Revision: 1.0 $
#
# DATED:              $Date: 2015-10-1 11:16:28 +0000 $
#
# AUTHOR:             PCT
#
###########################################################################
#
# Copyright Tonly B.V. 2015. All rights reserved
#
###########################################################################
TARGET := iotc_net_daemon

SRC_DIR := .
SOURCE := $(wildcard $(SRC_DIR)/*.c)

CFLAGS := -I./include/
PROJ_DFLAGS := -D_REENTRANT
PROJ_LIBS := -L./lib
PROJ_LIBS += -lpthread -ljson-c -lsqlite3

CC ?= gcc
RM ?= -rm
CFLAGS += -ggdb -Wno-unused-but-set-variable -Wall -O0 

OBJS := $(patsubst %.c,%.o,$(SOURCE))

vpath %.c $(SRC_DIR)

.PHONY: all clean distclean
all: $(TARGET)
	
$(TARGET):$(OBJS)
	@$(CC) $(PROJ_DFLAGS) $^ $(PROJ_CFLAGS) $(CFLAGS) $(PROJ_LIBS) -o $@ 
	
%.o:%.c
	$(CC) $(PROJ_DFLAGS) -I. $(CFLAGS) -c $< -o $@

%d:%c
	@set -e;$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's/.o:/.o $@:/g' < $@.$$$$ > $@; \
	rm $@.$$$$
-include $(SOURCE:.c=.d)

test:
	@echo $(SOURCE)
	@echo $(OBJS)
	
clean:
	$(RM) $(TARGET) $(OBJS) $(DEPS) *.d* 
 


