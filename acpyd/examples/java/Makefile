JFLAGS = -g
JC = javac
.SUFFIXES: .java .class
.java.class:
	$(JC) $(JFLAGS) $*.java

CLASSES = \
		  TestScriptServer.java \
		  TestScriptClient.java

all: classes

classes: $(CLASSES:.java=.class)

doc:
install:
clean:
	$(RM) -f *.class
