# Copyright (c) 2019 Daniel Abrecht
# SPDX-License-Identifier: WTFPL OR MIT

CC = gcc
BUILD = build
TARGET = lib/libdpaparser.a
PREFIX = /usr/local

INSTALL_FILES += $(TARGET)
INSTALL_FILES += bin/dpaparser
INSTALL_FILES += include/dpaparser/

INCLUDES += -Iinclude

OPTIONS = -std=c99 -Wall -Werror -Wextra -pedantic
OPTIONS += $(INCLUDES)

SOURCES = src/yamlparser.c
OBJECTS = $(addsuffix .o,$(addprefix $(BUILD)/,$(SOURCES)))

all: $(TARGET)

$(BUILD)/%.c.o: %.c
	@mkdir -p "$(dir $@)"
	$(CC) $(OPTIONS) -c $< -o $@

$(TARGET): $(OBJECTS)
	@mkdir -p "$(dir $@)"
	rm -f "$@"
	ar crs "$@" $^

clean:
	rm -rf $(BUILD) $(TARGET)

install: $(INSTALL_FILES)
	for file in $(INSTALL_FILES); \
	  do cp -r $$file $(PREFIX)/$$file; \
	done

uninstall:
	for file in $(INSTALL_FILES); \
	  do rm -r $(PREFIX)/$$file; \
	done
