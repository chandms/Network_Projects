Required Files :
	1. server.c
	
	Functions: security_check -> checks for valid IP address & then check for received message. Returns 1 if both are valid, otherwise returns -1.
		   parse -> to parse the received command and tokenize which is used as argument in execvp
		   main -> takes care of the rest of the functionality
		   
	2. client.c
	
	Functions: initiate_socket() -> initiate a socket
		   close_socket() -> close socket connection
		   connect_to_socket() -> connects with server
		   term_prog() -> signal handler which is activated if the response doesn't arrive from server
		   main -> takes care of the rest of the functionality
		   
	makefile:
		creates rcommandserver.bin and rcommandclient.bin
		   
