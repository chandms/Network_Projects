Required Files:
	1. server.c 
		a. main function -> which takes care of the creation of socket, connecting with clients and dealing with messages
		
	2. client.c
		a. send_message_to_server() -> to send message from client to server
		b. term_prog() -> this is activated 10seconds after client sends the last request
		c. some_prog() -> after client sends one request (not the last request), this function ensures that the client will wait for T seconds 
		                  and send next request irrespective of the previous response from server.
		d. main() -> takes care of socket creation, connection with server and the rest of the process
		
	3. makefile 
		creates mypingcli, mypingsrv
