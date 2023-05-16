CXX = clang++
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Werror $(FLAGS)

SOURCE=deque_tests.cpp
OUT=a.out
OUTPUT=output.txt

build:
	$(CXX) $(CXXFLAGS) $(SOURCE)

run: $(OUT)
	./$(OUT) > $(OUTPUT)

valgrind: $(OUT)
	valgrind ./$(OUT) $(FLAGS) > $(OUTPUT)

clean:
	rm -rf $(OUT) $(OUTPUT)

