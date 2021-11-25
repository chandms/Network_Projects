all:	program1	program2
program1:	server_conc.c
		gcc server_conc.c -o audiosrv -lasound -lm -lpthread

program2:	client_conc.c
		gcc client_conc.c -lm -o audiocli -lasound -lm -lpthread
