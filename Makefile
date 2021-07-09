project = ext++
version = 0.1.0

STD := c++2a

library = lib$(project)

install := $(library)
targets := $(install)

$(library).type = shared
define $(library).libs
 fmt
endef

define test.libs
 $(project)
 gtest
 gtest_main
endef
test.deps = $(library)

include mkbuild/base.mk
