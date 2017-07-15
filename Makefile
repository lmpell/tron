#Makefile

all: tron 

tron: tron.c set_ticker.c -lrt -lcurses
	gcc -o tron tron.c set_ticker.c -lrt -lcurses

clean:
	rm tron
	rm *.txt

run: 
	./tron
