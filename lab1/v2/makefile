all:	program1	program2	program3	program4
program1:	parser.c
		gcc -Wall -c parser.c

program2:	server.c
		gcc -Wall -c server.c

program3: 
		gcc -o commandserver.bin parser.o server.o

program4:
		gcc client.c -o commandclient.bin