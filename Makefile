
# Compiler
CC := gcc

# Package name
pkg_name := libCLI

# Source code directory
src_dir := src

# Build directory
build_dir := build

# Compiler flags
CFLAGS :=

# Defines
DEFINES :=

# Linker flags
LDFLAGS :=

# Other libraries
LDLIBS := -ljansson

# Includes
includes := -Isrc

# Source files
sources := $(src_dir)/cli.c \
		   \

# Object files
objects := $(patsubst %.c, $(build_dir)/%.o, $(sources))

all: $(objects)

$(build_dir)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(includes) -o $@ -c $<

# Test sources
test_src_dir := test

# Build directory for unit tests
test_build_dir := $(build_dir)/test

# unit test sources
test_sources := $(test_src_dir)/example.c \
                \

#unit test object files
test_objects := $(patsubst %.c, $(test_build_dir)/%.o, $(test_sources))

test_exec := $(test_build_dir)/example

test: $(test_exec)

$(test_exec): $(test_objects) $(objects)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(test_build_dir)/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(includes) -o $@ -c $<

clean:
	rm -rf $(build_dir)

.PHONY: all clean

