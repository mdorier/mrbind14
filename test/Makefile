INC=-I$(MRUBY)/include -I../include
LIB=-lmruby

all: test.cpp
	$(CXX) -o main test.cpp $(INC) $(LIB) -std=c++14

clean:
	rm main
