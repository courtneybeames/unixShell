.phony all:
all: PMan

PMan: PMan.c
	gcc PMan.c -o PMan

clean:
	-rm -f PMan.o PMan
