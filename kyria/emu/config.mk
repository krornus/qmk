CC := gcc

TARGET := oled

OBJ := qmk.o ../keymap.o ../ripple.o
DEP := $(OBJ:.o=.d)

CFLAGS := -std=c99
CFLAGS += -Wall -Wpedantic -Wextra
CFLAGS += -MMD
CFLAGS += -D_GNU_SOURCE -DQMK_EMULATOR
CFLAGS += -DMATRIX_ROWS=4 -DMATRIX_COLS=8
CFLAGS_DEBUG   := -O0 -ggdb
CFLAGS_RELEASE := -O2

ifeq ($(DEBUG),1)
CFLAGS += $(CFLAGS_DEBUG)
else
CFLAGS += $(CFLAGS_RELEASE)
endif

LDLIBS := -lSDL2
LDFLAGS :=
