LIBS = -pthread
FLAGS = -Wall -Werror -std=c++11


src = $(wildcard *.cpp)
objects = $(patsubst %.cpp, %.o, $(src))


all:libReactor.a 

libReactor.a:$(objects)
	 ar -cr  $@ $(objects) 


$(objects):%.o : %.cpp 
	g++ -c $(FLAGS)  $<  -o $@


.PHONY:test
test:
	cd test && $(MAKE)


.PHONY:clean
clean:
	-rm -f *.o




