###########################################################################
#
# MODULE:             IOTC - Makefile
#
# REVISION:           $Revision: 1.0 $
#
# DATED:              $Date: 2015-10-28 11:16:28 +0000 $
#
# AUTHOR:             Abel
#
###########################################################################
#
# Copyright panchangtao@gmail.com B.V. 2015. All rights reserved
#
###########################################################################
TARGET := iotc_client

SRC_NAME := $(shell pwd)
SRC_DIR := $(SRC_NAME)/src
INC_DIR := $(SRC_NAME)/inc
LIB_DIR := 

SOURCE := $(wildcard $(SRC_DIR)/*.c)
INCLUD := $(wildcard $(INC_DIR)/*.h)

CFLAGS := -I./inc/
CFLAGS += -I$(SRC_NAME)/inc/
PROJ_DFLAGS := -D_REENTRANT
PROJ_LIBS := -L$(LIB_DIR)
PROJ_LIBS += -lpthread -ljson-c

CC := gcc
RM := -rm
CFLAGS += -O2 -g -Wno-unused-but-set-variable -Wall
#CFLAGS += -O2 -g -Wcomment -Wformat -Wimplicit -Wreturn-type



OBJS := $(patsubst %.c,%.o,$(SOURCE))
INCLUDE := $(patsubst %.c,%.h,$(INCLUD))

vpath %.c $(SRC_DIR)

.PHONY: all clean distclean
all: $(TARGET)
	
include $($(SOURCE):.c=.d)
	
$(TARGET):$(OBJS)
	@$(CC) $(PROJ_DFLAGS) $^ $(PROJ_CFLAGS) $(CFLAGS) $(PROJ_LIBS) -o $@ 
	
%.o:%.c $(INCLUDE) 
	$(CC)  -I. $(CFLAGS) $(PROJ_CFLAGS) -c $< -o $@

	
test:
	@echo $(SOURCE)
	@echo $(CC)
	@echo $(OBJS)
	
clean:
	$(RM) $(TARGET) $(OBJS)

distclean:
	$(RM) $(TARGET) $(OBJS)
 


