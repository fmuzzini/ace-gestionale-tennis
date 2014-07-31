VPATH = src/
OBJ = ACE.o accesso_dati.o
LIBRERIE = gtk+-3.0
LIBS = `pkg-config --libs $(LIBRERIE)`
FLAGS = `pkg-config --cflags $(LIBRERIE)`
CXXFLAGS = -Wall $(FLAGS)


ACE: $(OBJ)
	g++ -o ACE $(OBJ) $(LIBS)

-include dependencies

.PHONY: depend clean cleanall debug

depend:
	g++ -MM $(VPATH)*.cc > dependencies

debug: CXXFLAGS += -g -D DEBUG_MODE
debug: ACE

clean:
	rm *.o -f
cleanall:
	rm ACE *.o -f
