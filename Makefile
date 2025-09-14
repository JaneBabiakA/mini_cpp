CXX = g++
CXXFLAGS = -std=c++20 `llvm-config --cppflags`
LDFLAGS = `llvm-config --ldflags --system-libs --libs all`
SRC = src/main.cpp src/frontend/request.cpp src/frontend/lexer.cpp src/frontend/parser.cpp src/backend/codegen.cpp
TARGET = mini_cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)