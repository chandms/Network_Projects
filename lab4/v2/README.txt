1. server.c

	Functions:
		a. ip_address_check(struct sockaddr_in) -> it will check if the message is coming from valid ip address
		
		b. send_file_info_to_client() -> send file content to client (window logic)
		
		c. wait_ack(int ) -> signal handler after timeout
		
		d. unsigned int bbencode(unsigned int res, int pub) -> does XOR operation on the result obatined from client and public key.

		e. main() -> creates socket, accepts requests from client, read request, do ip address check, 
				then checks for authentication. pass the unsigned int obtained from client to bbencode(). If the result matches client ip, then authentication is successful.
				Then it checks the file name.
			       if file exists, then read open the file and call send_file_info_to_client() function.


2. client.c

	Functions:
		a. initiate_socket() -> to initiate socket
		b. close_socket() -> to close socket connection
		c. connect_to_socket() -> connect to the server
		d. term_prog (int ) -> signal handler to resend request
		e. check_param() -> to check if the secert key is within range, the file name given is proper.
		f. unsigned int bbdecode(int secret_key, unsigned int client_ip) -> XOR the client ip and private key given by the user
		g. check_host_name() -> to check if host name valid
		h. check_host_entry() -> check if host entry is valid
		i. unsigned int convert() -> convert ip address to 32 bit integer [ A.B.C.D -> A*(256)^3+B*(256)^2+C*(256)+D]
		i. main() -> will prepare the message, send request to server, store the response file in "Client" directory within v2 folder, Calculates throughput.


		Point to note : rrunner will store the response file within Client directory in v2 folder.
				 my acl.dat file looks like below -
				 	128.10.25.222 123
				 	128.10.25.234 564
				 	128.10.112.455 100
				 	..................
				 	
				 	
				 	
				
