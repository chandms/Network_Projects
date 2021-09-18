README for Problem3 - Client/Server App

Files :

	1. parser.h : parser header file
	2. parser.c : parser definition file 
				  parse function parses the command line input according to the provided delimiter.
	3. server.c : It has main function which creates 			   serverfifo.dat file.
				  server writes the fifo name in the above mentioned file.
				  server reads requests from clients and execute execvp.
				  writes the output on specific client 
				  fifo.
	4. client.c : It has main function which first 				  tries to read the serverfifo.dat, if 				 not available, prints error to stdout

				  If client reads the fifo name, it gets commandline input from user and writes the command to fifo.

				  client waits for the reponse from server.
				  clients reads the response written by server in the specific client fifo.

	5. makefile : It has all the commands to generate 
				   commandclient.bin, commandserver.bin