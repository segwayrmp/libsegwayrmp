all: segwayrmp

install:
	cd build && make install

uninstall:
	cd build && make uninstall

segwayrmp:
	@mkdir -p build
	-mkdir -p bin
	cd build && cmake $(CMAKE_FLAGS) -DSEGWAYRMP_BUILD_EXAMPLES=1 ..
ifneq ($(MAKE),)
	cd build && $(MAKE)
else
	cd build && make
endif

clean:
	-cd build && make clean
	rm -rf build bin lib

.PHONY: doc
doc:
	@doxygen doc/Doxyfile
ifeq ($(UNAME),Darwin)
	@open doc/html/index.html
endif

.PHONY: test
test:
	@mkdir -p build
	@mkdir -p bin
	cd build && cmake $(CMAKE_FLAGS) -DSEGWAYRMP_BUILD_TESTS=1 -DSEGWAYRMP_BUILD_EXAMPLES=1 ..
ifneq ($(MAKE),)
	cd build && $(MAKE)
else
	cd build && make
endif
	@if test -e bin/segwayrmp_tests; then bin/segwayrmp_tests; fi
