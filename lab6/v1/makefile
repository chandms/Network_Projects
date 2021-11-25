all:	program1	program2	program3	program4	program5
program1:	zigzagconf.c
		gcc -Wall -c zigzagconf.c -o zigzagconf -lm 
program2:	server.c
		gcc server.c -o mypingsrv

program3:	client.c
		gcc -Wall -c client.c -o c.out -lm
program4: 	
		gcc -o mypingcli c.out zigzagconf -lm
program5:
		gcc zigzagrouter.c -o zigzagrouter -lm