CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -fPIC -pthread
LDFLAGS = -shared
INSTALL_DIR = /usr/local/lib

all: libcaesar.so secure_copy test_caesar

libcaesar.so: pr1.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o libcaesar.so pr1.cpp

secure_copy: secure_copy.cpp caesar.h libcaesar.so
	$(CXX) $(CXXFLAGS) -o secure_copy secure_copy.cpp -L. -lcaesar -Wl,-rpath,'$$ORIGIN'

install: libcaesar.so
	cp libcaesar.so $(INSTALL_DIR)/
	@which ldconfig > /dev/null 2>&1 && ldconfig || true

test: test_caesar libcaesar.so
	./test_caesar ./libcaesar.so X input.txt output.txt

test_secure_copy: secure_copy
	./secure_copy input.txt output.txt X

test_caesar: test_caesar.cpp
	$(CXX) $(CXXFLAGS) -o test_caesar test_caesar.cpp -ldl

clean:
	rm -f libcaesar.so secure_copy test_caesar output.txt

.PHONY: all install test test_secure_copy clean
