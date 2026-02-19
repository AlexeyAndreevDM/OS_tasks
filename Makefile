CXX = g++
CXXFLAGS = -Wall -Wextra -pedantic -fPIC
LDFLAGS = -shared
INSTALL_DIR = /usr/local/lib

all: libcaesar.so

libcaesar.so: pr1.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o libcaesar.so pr1.cpp

install: libcaesar.so
	cp libcaesar.so $(INSTALL_DIR)/
	@which ldconfig > /dev/null 2>&1 && ldconfig || true

test: test_caesar libcaesar.so
	@echo "A" > input.txt
	./test_caesar ./libcaesar.so X input.txt output.txt

test_caesar: test_caesar.cpp
	$(CXX) $(CXXFLAGS) -o test_caesar test_caesar.cpp -ldl

clean:
	rm -f libcaesar.so test_caesar output.txt

.PHONY: all install test clean
