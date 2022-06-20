project = ext++

STD := c++20

library = lib$(project)

install := $(library)
targets := $(install)

$(library).type = shared
$(library).libs = fmt

test.libs = $(project) gtest gtest_main
test.deps = $(library)

install.directories = $(include)/ext

files = $(include) $(src) Makefile VERSION

include mkbuild/base.mk
