zzoverlay.dat structure:
	4 
	128.10.25.224 33333
	55001 39001 128.10.25.214
	50003 0 0.0.0.0  
	128.10.25.214 33333
	39001 55001 128.10.25.215
	50004 50005 128.10.25.215
	128.10.25.215 33333
	55001 39001 128.10.25.222
	50005 50003 128.10.25.224
	128.10.25.222 33333
	39001 55001 128.10.25.221
	40001 50004 128.10.25.214

	## Note -> the first hop's return path must be given as 0 0.0.0.0 ( return address is 0.0.0.0 and port is 0 for 128.10.25.224)
	## Note -> Hops are considered in sequential order (here 128.10.25.224 is the first hop)


zigzagrouter.c

	(Run this first in all hops as per zzoverlay.dat file)
	

	1. void close_zig_socket(int sock) : to close the given socket

	2. int create_zig_socket(int sock) : to initiate the given socket

	3. int bind_to_zig_socket(int sock, int sock_num) : bind the given socket

	4. int main(int argc, char* argv[]) : 
		a. gets the udp_management_packet sent by zigzagconf
		b. prints the routing table
		c. creates two sockets - forward and backward
		d. selects backward and forward sockets to send information to other machines




zigzagconf.c

	(Run this in separate host)

	1. void close_the_socket() : to close the socket

	2. void initiate_the_socket() : create the socket

	3. void bind_to_the_socket() : bind to socket

	4. void parse(char line[100], char delim[2]) : parse the line according to delimeter

	5. int main() : 
		a. reads zzoverlay.dat
		b. creates udp management packet
		c. sends the packet to associated hops


server.c 
		1. main function -> which takes care of the creation of socket, connecting with clients and dealing with messages
		
client.c
		1. send_message_to_server() -> to send message from client to server

		2. term_prog() -> this is activated 10seconds after client sends the last request

		3. some_prog() -> after client sends one request (not the last request), this function ensures that the client will wait for T seconds and send next request irrespective of the previous response from server.

		4. main() -> takes care of socket creation, connection with server and the rest of the process
		
makefile 
		creates mypingcli, mypingsrv
		creates zigzagrouter, zigzagconf





