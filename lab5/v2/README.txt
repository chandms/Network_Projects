Note : sample command line argument:
	a. audiosrv 128.10.25.202 30000 313 log_serv

	b. audiocli 128.10.25.202 30000 pp.au 4096 12288 6144 313 1 log_pp 
	
Note : we take arguments in miliseconds, then convert it accordingly

1. server_conc.c

	Functions:
		a. ip_address_check(struct sockaddr_in) -> it will check if the message is coming from valid ip address
		b. mulawopen -> audio device open with specific block size
		c. mulaclose -> close the audio device
		d. main-> we create the socket to connet with clients. Then if information received  (block size and file name) from client is authentic, Then
				  server forks a child and transfer the audio data to the client. Also, the child waits to receive acknowledgement from the client.
				  The acknowledgement is the new updated lambda which we store in the array corresponding to the time.
				  After all the file has been transferred, the child writes all the lambda values and timestamps in file name given as command line input 
				  corresponding to the client_i 

2. client_conc.c

	Functions:
		a. mulaclose -> close the audio device
		b. mulawopen -> audio device open with specific block size
		c. mulawwrite -> writing to audio device
		e. update_lambda- to update the lambda value according to the given method id in command line arguments 0-Method C, 1- Method , 2- Method E
		c. produce- part of the implementation of producer consumer methodology; reads data from server if there is capacity in the buffer
		d. consume()- part of the implementation of producer consumer methodology; writes to audio device if there is enough data (>=block_size) in buffer
		e. initiate_socket() -> to initiate socket
		f. close_socket() -> to close socket connection
		g. bind_to_socket() -> bind to sockett
		h. check_param() -> to check if the secert key is within range, the file name given is proper.
		. main() -> will prepare the message, send request to server, 
					conumer writes the audio data to device. after getting the full file, the client writes its buffer
					occuoancies throughout the process in log file.
