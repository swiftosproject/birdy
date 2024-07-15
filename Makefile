CXX = g++

CXXFLAGS = -g -O0

LIBS = -lcurl -ljsoncpp -larchive

SRC = frontend.cpp
TARGET = birdy

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LIBS)

chmod: $(TARGET)
	chmod +x $(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

install:
	cp birdy /usr/bin/birdy
	chmod +x /usr/bin/birdy

.PHONY: all chmod run clean
