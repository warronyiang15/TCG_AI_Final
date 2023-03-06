all:
	g++ -O3 -std=c++11 -Wno-deprecated-declarations -o main src/ver18/main.cpp src/ver18/MyAI.cpp
clean:
	rm -f main
