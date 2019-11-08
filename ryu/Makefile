# This makefile builds libryu.a. Mostly useful for building
# the lib as a dependency in non-Bazel projects (e.g. CMake).
# Contributed by @gritzko. Supported on a best-effort basis.

SRC=d2fixed.c d2s.c f2s.c generic_128.c

OBJ = $(SRC:.c=.o)

ALIB = libryu.a

INCLUDES = -I..

.SUFFIXES: .c

default: $(ALIB)

.c.o:
	$(CC) $(INCLUDES) $(CFLAGS) -c $< -o $@

$(ALIB): $(OBJ)
	$(AR) rcs $(ALIB) $(OBJ)

libs: $(ALIB)

.PHONY: install
install: libs
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include
	cp $(ALIB) $(DESTDIR)$(PREFIX)/lib/$(ALIB)
	cp ryu.h $(DESTDIR)$(PREFIX)/include/

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/lib/$(ALIB)
	rm -f $(DESTDIR)$(PREFIX)/include/ryu.h

TESTSRC=tests/common_test.cc tests/d2fixed_test.cc tests/d2s_intrinsics_test.cc tests/d2s_table_test.cc tests/d2s_test.cc tests/f2s_test.cc tests/generic_128_test.cc

TESTS = $(TESTSRC:.cc=.test)

%.test: %.cc $(ALIB)
	$(CXX) -L/usr/lib64 $< ../third_party/gtest/gtest-all.cc ../third_party/gtest/gtest_main.cc -lpthread -I.. $(ALIB) -frtti -o $@
	$@

.PHONY: test
test: $(TESTS)
	@echo DONE

.PHONY: clean
clean:
	rm -f $(OBJ) $(ALIB) $(TESTS)

