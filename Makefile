all: myprime innerChild leafChild

myprime:
	gcc myprime.c list.c -o myprime -lm

innerChild:
	gcc innerChild.c list.c -o innerChild -lm

leafChild:
	gcc leafChild.c -o leafChild -lm

clean:
	rm -f myprime innerChild leafChild
	

