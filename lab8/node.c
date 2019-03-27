//Taken from Abhishek Sagar

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <arpa/inet.h>

/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT     2000 
#define SERVER_IP_ADDRESS   "10.20.1.22"

char data_buffer[1024];
char filename[100] = "file.txt";


void setup_tcp_server_communication(){
   int master_sock_tcp_fd = 0, 
       sent_recv_bytes = 0, 
       addr_len = 0, 
       opt = 1;
   int comm_socket_fd = 0;
   fd_set readfds;             
   struct sockaddr_in server_addr, 
                      client_addr;
   if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1){
       printf("socket creation failed\n");
       exit(1);
   }
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = SERVER_PORT;
   server_addr.sin_addr.s_addr = INADDR_ANY; 
   addr_len = sizeof(struct sockaddr);
   if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1){
       printf("socket bind failed\n");
       return;
   }

   struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    if (getsockname(master_sock_tcp_fd, (struct sockaddr *)&sin, &len) == -1)
        perror("getsockname");
    else
        printf("port number %d\n", ntohs(sin.sin_port));

   if (listen(master_sock_tcp_fd, 5)<0) {
       printf("listen failed\n");
       return;
   }
	while(1){
		FD_ZERO(&readfds);                     
       	FD_SET(master_sock_tcp_fd, &readfds);  
       	printf("blocked on select System call...\n");
       	select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL); 
       	if (FD_ISSET(master_sock_tcp_fd, &readfds)){ 
        	printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");
           	comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
           	if(comm_socket_fd < 0){
            	printf("accept error : errno = %d\n", errno);
                exit(0);
           	}        
           	// SERVER ESTABLISHED CONNECTION WITH NEW NODE
           	printf("Got new node! 'NAME':%s:%u\n",
                      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
           	while(1){
               	printf("Server ready to service client msgs.\n");
               	// RECEIVE FILENAME FROM CLIENT
               	sent_recv_bytes = recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0,
                       (struct sockaddr *)&client_addr, &addr_len);
               	char result[10] = "no";
               	printf("Server got: %s\n", data_buffer);
               	int fd = open(data_buffer, 2);
               	if (fd > 0){
               		strcpy(result, "yes 2");
               	}
               	// SEND RESPONSE WITH yes/no & # of Words IN FILE
               	sent_recv_bytes = sendto(comm_socket_fd, (char *)&result, sizeof(char)*10, 0,
                       (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
               	printf("Server -> client: %s\n", result);

               	while(1){
               		// TODO: this loop should be executed only when (fd > 0)
               		// 		& # of iterations no more then words in file
               		// RECEIVE # OF WORD TO SEND
               		sent_recv_bytes = recvfrom(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0,
                    	   (struct sockaddr *)&client_addr, &addr_len);
               		printf("Server got:%s\n", data_buffer);

               		// SEND CURRENT FILE WORD TO CLIENT
               		char current_word[20] = "next_word";
               		sent_recv_bytes = sendto(comm_socket_fd, (char *)&current_word, sizeof(char)*20, 0,
                    	   (struct sockaddr *)&client_addr, sizeof(struct sockaddr));
               		printf("Server -> client: %s\n", current_word);
               		printf("No of bytes sent = %d\n", sent_recv_bytes);


               }
           }
       }
   }
}

void setup_tcp_client_communication() {
    int sockfd = 0, 
    sent_recv_bytes = 0;
    int addr_len = 0;
    addr_len = sizeof(struct sockaddr);
    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
	dest.sin_port = SERVER_PORT;
    struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// CONNECT TO THE SERVER
    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    while(1) {		

    	// SEND FILENAME TO SERVER
      	char send_msg[100];
      	printf("check if server has file\n");
      	strcpy(send_msg, filename);
      	sent_recv_bytes = sendto(sockfd, &send_msg, sizeof(send_msg),
      							0, (struct sockaddr *)&dest, sizeof(struct sockaddr));
      	printf("No of bytes sent = %d\n", sent_recv_bytes);

      	// GET THE ANSWER FROM SERVER IF IT HAS FILE & # of words in file
      	char result2[10];
      	sent_recv_bytes =  recvfrom(sockfd, (char *)&result2, sizeof(char)*10, 
      								0, (struct sockaddr *)&dest, &addr_len);
      	printf("No of bytes received = %d: %s\n", sent_recv_bytes, result2);
      	char if_file_exist[4] = "string";
      	int number_of_words = 10;
      	sscanf(result2, "%s %d", if_file_exist, &number_of_words);
      	printf("%s %d\n", if_file_exist, number_of_words);


      	if (strcmp(if_file_exist, "yes") == 0){
      		printf("file exist, request for download, #ofWords: %d\n", number_of_words);
      		int i = 0;
      		char result3[50] = "string";

      		// REQUEST i'th WORD FROM FILE FROM SERVER
      		while (i < number_of_words){
      			// CONVERT i TO STRING msg
      			snprintf(send_msg, 3, "%d", i);
      			
      			// SEND i TO SERVER
      			printf("request %s'th word\n", send_msg);
      			sent_recv_bytes = sendto(sockfd, &send_msg, sizeof(char)*3,
      							0, (struct sockaddr *)&dest, sizeof(struct sockaddr));


      			// RECEIVE THE i'th WORD ROM SERVER
      			// -! size of receive buffer should be greater than sent buffer of server
      			sent_recv_bytes =  recvfrom(sockfd, (char *)&result3, sizeof(char)*50, 
      								0, (struct sockaddr *)&dest, &addr_len);
      			printf("client got: %s\n", result3);
      			printf("No of bytes recv = %d\n", sent_recv_bytes);
      			i++;
      		}



      	} else {
      		printf("server has no file\n");
      	}

      	exit(1);
      
    }
}



int main(int argc, char **argv){
  char option = 'c';
  while(1){
    printf("Run as a client or as a server?(c|s)\n");
    scanf("%c", &option);
    if(option == 's'){
      setup_tcp_server_communication();
    }
    else if (option = 'c'){
      setup_tcp_client_communication();
    } else {
      printf("Wrong option. (type only one character)\n");
    }
  }
  
  return 0;
}