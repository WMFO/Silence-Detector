IFLAGS=-I./inc -Ilib/rapidxml-1.13
LIBS =-lcurl
OBJDIR=bin
SRCDIR=src
OBJS=bin/silenceException.o bin/detectorTypes.o bin/silenceDetector.o bin/RMSMeasurement.o bin/streamDumper.o

default: silenceDetector

$(OBJDIR) : 
	mkdir $(OBJDIR)

$(OBJDIR)/%.o : $(SRCDIR)/%.cpp $(OBJDIR)
	g++ -c $(IFLAGS) $< -o $@

test : $(OBJS) bin/testMain.o
	g++  -o $(OBJDIR)/test $^ $(LIBS)

clean:
	rm $(OBJDIR)/*

silenceDetector: bin/silenceException.o bin/detectorTypes.o bin/silenceDetector.o bin/RMSMeasurement.o bin/streamDumper.o bin/main.o
	g++ -o $(OBJDIR)/silenceDetector $^ $(LIBS)

