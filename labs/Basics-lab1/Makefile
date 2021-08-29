all: build

build: cat tac

cat: cat.c	
	gcc cat.c -o cat

tac: tac.c
	gcc tac.c -o tac

run: cat tac
	./cat
	./tac

clean:
	rm cat
	rm tac