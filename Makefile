all: server

clean:
	@rm -rf *.o
	@rm -rf server

server: ./objs/main.o ./objs/httpd.o ./objs/chain.o  ./objs/functions.o ./objs/router.o ./objs/node.o
	gcc -o server $^  -lssl -lcrypto 

./objs/main.o: main.c httpd.h chain.h node.h
	gcc -c -o ./objs/main.o main.c

./objs/httpd.o: httpd.c httpd.h chain.h node.h
	gcc -c -o ./objs/httpd.o httpd.c

./objs/chain.o: chain.c chain.h
	gcc -c -o ./objs/chain.o chain.c

./objs/functions.o: functions.c functions.h
	gcc -c -o ./objs/functions.o functions.c -lssl -lcrypto 

./objs/router.o: router.c httpd.h chain.h functions.h
	gcc -c -o ./objs/router.o router.c

./objs/node.o: node.c node.h chain.h functions.h
	gcc -c -o ./objs/node.o node.c
