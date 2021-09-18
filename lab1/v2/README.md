README for Problem2 - Client/Server App

Files :

	1. parser.h : parser header file

	2. parser.c : parser definition file (parse function parses the command line input)

	3. server.c : It has main function which creates serverfifo.dat file.
	server writes the fifo name in the above mentioned file.
	server reads requests from clients and execute execvp and shows the output on server terminal.

	4. client.c : It has main function which first 	 tries to read the serverfifo.dat, if 				not available, prints error to stdout, exits.

	If client reads the fifo name, it gets commandline input from user and writes the command to fifo.

	5. makefile : It has all the commands to generate commandclient.bin, commandserver.bin