VPATH=..
LIBS = -pthread
FLAGS = -Wall -Werror -std=c++11

all=client server

dst:$(all)

$(all):% :%.cpp libReactor.a
	g++ $< -I.. -L.. -lReactor $(LIBS) $(FLAGS) -o $@


.PHONY:clean
	-rm -f $(all)

