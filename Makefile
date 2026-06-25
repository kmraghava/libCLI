
# Compiler
CC := gcc

# Package name
pkg_name := libCLI

# Source code directory
src_dir := src

# Build directory
build_dir := build

# Compiler flags
CFLAGS := -Wall -Wextra -g

# Defines
DEFINES := -D_GNU_SOURCE

# Linker flags
LDFLAGS :=

# Other libraries
LDLIBS := -ljansson -lkmrUtils

# Includes
includes := -Iinclude \
            -Iinclude/internal \
			\

# Source files
sources := $(src_dir)/cli_builder.c \
		   $(src_dir)/cli_io.c \
		   $(src_dir)/cli_log.c \
		   $(src_dir)/cli_parser.c \
		   $(src_dir)/cli_vvalidator.c \
		   $(src_dir)/cli.c \
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
example_sources := $(test_src_dir)/example.c \
                   \

cisco_ios_simulator_sources := $(test_src_dir)/cisco_ios/cisco_ios_config_t.c \
							   $(test_src_dir)/cisco_ios/cisco_ios_interface.c \
							   $(test_src_dir)/cisco_ios/cisco_ios_main.c \
							   $(test_src_dir)/cisco_ios/cisco_ios_show.c \
                               \

#unit test object files
example_objects := $(patsubst $(test_src_dir)/%.c, $(test_build_dir)/%.o, $(example_sources))
cisco_ios_simulator_objects := $(patsubst $(test_src_dir)/%.c, $(test_build_dir)/%.o, $(cisco_ios_simulator_sources))

example_exec := $(test_build_dir)/example
cisco_ios_simulator := $(test_build_dir)/cisco_ios_simulator

test: $(example_exec) $(cisco_ios_simulator)

$(example_exec): $(example_objects) $(objects)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(cisco_ios_simulator): $(cisco_ios_simulator_objects) $(objects)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

$(test_build_dir)/%.o: $(test_src_dir)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(DEFINES) $(includes) -o $@ -c $<

clean:
	rm -rf $(build_dir)

.PHONY: all clean

