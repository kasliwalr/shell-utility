
bin/wish: src/wish.o
	gcc -o bin/wish src/wish.o 

src/wish.o: src/wish.c
	gcc -o src/wish.o -c src/wish.c -g -Wall -Werror 
clean: 
	rm -f bin/* src/*.o 
