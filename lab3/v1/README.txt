1. server.c

	Functions:
		a. ip_address_check(struct sockaddr_in) -> it will check if the message is coming from valid ip address

		b. main() -> creates socket, it will continue accepting requests from client, read request, process security checks, then if the security checks pass, it will fork one child to read the file and send the data to client.


2. client.c

	Functions:
		a. initiate_socket() -> to initiate socket
		b. close_socket() -> to close socket connection
		c. connect_to_socket() -> connect to the server
		d. term_prog (int ) -> signal handler to resend request
		e. check_param() -> checks the command input provided by the user. checks the secret key and filename.
		f. main() -> will prepare the message, send request to server, store the response file in "Client" directory within v1 folder.


		Point to note : myftpc will store the response file within Client directory in v1 folder.
