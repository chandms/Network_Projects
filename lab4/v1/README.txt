1. server.c

	Functions:
		a. ip_address_check(struct sockaddr_in) -> it will check if the message is coming from valid ip address
		
		b. send_file_info_to_client() -> send file content to client (window logic)
		
		c. wait_ack(int ) -> signal handler after timeout

		d. main() -> creates socket, accepts requests from client, read request, do ip address check, then do secret key match, then it checks the file name.
			     if file exists, then read open the file and call send_file_info_to_client() function.


2. client.c

	Functions:
		a. initiate_socket() -> to initiate socket
		b. close_socket() -> to close socket connection
		c. connect_to_socket() -> connect to the server
		d. term_prog (int ) -> signal handler to resend request
		e. check_param() -> to check if the secert key is within range, the file name given is proper.
		f. main() -> will prepare the message, send request to server, store the response file in "Client" directory within v1 folder.


		Point to note : rrunner will store the response file within Client directory in v1 folder.
