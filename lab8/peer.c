//Taken from Abhishek Sagar

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"

#define MAX_WORD_LENGTH 20
/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT     2003

//Info for the CLient
#define SERVER_IP_ADDRESS   "192.168.4.2"

#define FILENAME_REQUEST "test.txt"

char data_buffer[1024];

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int amount_of_words(char* filename)
{
    FILE * fp = fopen(filename, "r");
	if (fp == NULL) return 1;
	char c;
	int count = 0;
	while((c = fgetc(fp)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
			++count;
		}
	}
    fclose(fp);
    return (count + 1);
}

char** convert_text_to_array(char* filename){
    int size = amount_of_words(filename);
    FILE * fp = fopen(filename, "r");
    char **data = (char **)malloc(size * sizeof(char *));
		int i;
    for (i = 0; i < size; i++)
         data[i] = (char *)malloc(MAX_WORD_LENGTH * sizeof(int));
	if (fp == NULL) return 1;
	char c;
	int count = 0;
  i = -1;
	while((c = fgetc(fp)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
            data[count][++i] = '\0';
            count++;
            i = -1;
		}
		else
		{
            data[count][++i] = c;
		}
	}
	fclose(fp);
    return data;
}

void
tcp_server(){

   /*Step 1 : Initialization*/
   /*Socket handle and other variables*/
   /*Master socket file descriptor, used to accept new client connection only, no data exchange*/
   int master_sock_tcp_fd = 0,
       sent_recv_bytes = 0,
       addr_len = 0,
       opt = 1;

   /*client specific communication socket file descriptor,
    * used for only data exchange/communication between client and server*/
   int comm_socket_fd = 0;
   /* Set of file descriptor on which select() polls. Select() unblocks whever data arrives on
    * any fd present in this set*/
   fd_set readfds;
   /*variables to hold server information*/
   struct sockaddr_in server_addr, /*structure to store the server and client info*/
                      client_addr;



   /*step 2: tcp master socket creation*/
   //SOCK_STREAM to SOCK_DGRAM
   if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
   {
       printf("socket creation failed\n");
       exit(1);
   }

   /*Step 3: specify server Information*/
   server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
   server_addr.sin_port = SERVER_PORT;/*Server will process any data arriving on port no 2000*/


   server_addr.sin_addr.s_addr = INADDR_ANY;

   addr_len = sizeof(struct sockaddr);

   if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       printf("socket bind failed\n");
       return;
   }

   if (listen(master_sock_tcp_fd, 5)<0)
   {
       printf("listen failed\n");
       return;
   }


   while(1){

       FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
       FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

       printf("blocked on select System call...\n");


      select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

       if (FD_ISSET(master_sock_tcp_fd, &readfds))
       {

           printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");


           comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
           if(comm_socket_fd < 0){


               printf("accept error : errno = %d\n", errno);
               exit(0);
           }

           printf("Connection accepted from client : %s:%u\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

           printf("Got new node! 'NAME':%s:%u\n",
                      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

           while(1){

               printf("Server ready to service client msgs.\n");

               memset(data_buffer, 0, sizeof(data_buffer));

               sent_recv_bytes = recv(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0);


               if(sent_recv_bytes == 0){
               		close(comm_socket_fd);
               		break;
               }

							 char response[6] = "NOT OK";
							 char text_file[20];
							 strcpy(text_file, data_buffer);

               int fd = open(data_buffer, 2); //Заменить на файл поинтер
							 if(fd > 0 ){
								 printf("HAS THIS FILE \n");
								 strcpy(response, "OK");
							 }else{
								 printf("Error Reading\n");
							 }

							 send(comm_socket_fd, (char *)&response, sizeof(char)*6, 0);


               result_struct_t result;

							 char recieved_signal[5];

							 sent_recv_bytes = recv(comm_socket_fd, (char *)recieved_signal, sizeof(char)*5, 0);

							 if(strcmp(recieved_signal, "send")!=0){
								 	printf("%s not match `send`", recieved_signal);
								 	exit(-2);
 						  	}else{


									printf("Client said SEND file\n");

									//Заменить на передачу файл поинтера
									int size = amount_of_words(text_file);

							    //printf("%d", size);
							    char **data = convert_text_to_array(text_file);


									int word_num=0;
									int next_num = 0;
									char next_num_req[20];
									word_struct word_data;
									word_data.max_num = size - 2;

									int i=0;

									//pring all words
									// for (i = 0; i < size; i++){
									// 		printf("\n%s", data[i]);
									// }

									printf("Sending....\n");
									//loop of reading + sending
									while(fd > 0 && word_num < (size - 1)){

										sent_recv_bytes = recv(comm_socket_fd, (char *)next_num_req, sizeof(char)*20, 0);
									  next_num = atoi(next_num_req);

										//printf("Client -> Server: request next word number = %i\n", next_num);
										word_num = next_num;
										word_data.num = word_num;

										//printf("Server -> client: word='%s' word_number='%i'\n", data[word_num], word_num );
										strcpy(word_data.word, data[word_num]);

										send(comm_socket_fd, &word_data, sizeof(word_struct), 0);
										word_num++;
									}

 								}
								printf("TRANSFER COMPLETE\n");
								sleep(10);
           }
       }
   }
}


///////////// CLIENT /////////////////////
void tcp_client() {

    /*Step 1 : Initialization*/
    /*Socket handle*/
    int sockfd = 0,
        sent_recv_bytes = 0;

    int addr_len = 0;
    addr_len = sizeof(struct sockaddr);

    /*to store socket addesses : ip address and port*/
    struct sockaddr_in dest;

    /*Step 2: specify server information*/
    /*Ipv4 sockets, Other values are IPv6*/
    dest.sin_family = AF_INET;

    /*Client wants to send data to server process which is running on server machine, and listening on
     * port on DEST_PORT, server IP address SERVER_IP_ADDRESS.
     * Inform client about which server to send data to : All we need is port number, and server ip address. Pls note that
     * there can be many processes running on the server listening on different no of ports,
     * our client is interested in sending data to server process which is lisetning on PORT = DEST_PORT*/
    dest.sin_port = SERVER_PORT;
		struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
    dest.sin_addr = *((struct in_addr *)host->h_addr);

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));

    while(1) {

			char request_file[8] = "test.txt";
			char answer[6];


			int next_word = 0;


	    sent_recv_bytes = send(sockfd,
		   &request_file,
		   sizeof(request_file),  //May be not that
		   0);



	    printf("Requested: %s | No of bytes sent = %d\n", request_file, sent_recv_bytes);

			//Wait OK signal
	    sent_recv_bytes =  recv(sockfd, (char*)&answer, sizeof(char)*3, 0);

			//Compare OK with recieved message
			if(strcmp(answer, "OK")==0){
				printf("ANSWER: %s \n", answer);
		    printf("SERVER HAS THIS FILE\n");
		  }else{
				printf("Server said not ok :(\n");
		    exit(-2);
			}


			char send_signal[5] = "send";

			sent_recv_bytes = send(sockfd, &send_signal, sizeof(char)*5, 0);

	    printf("Requested: %s | No of bytes sent = %d\n", send_signal, sent_recv_bytes);

			FILE* fp = fopen(FILENAME_REQUEST, "w"); //create new file and open

			if(fp == NULL) {printf("Unenable to create file"); exit(-3); }

			char buffer[100];

			int word_num=0;
			char num_buf[20];
			word_struct word_data;
			int max_num = 100;

			//zaLoop Recieving word + number
			while(word_num <= max_num){

						//Request next word
					// CONVERT i TO STRING msg
					snprintf(num_buf, 3, "%d", word_num);
					//printf("Send next number: %s\n", num_buf);
					sent_recv_bytes = send(sockfd, (char*)&num_buf, sizeof(char)*20, 0);


				//printf("wait for the word\n");


				sent_recv_bytes =  recv(sockfd, (char*)buffer, sizeof(buffer), 0);

				word_struct *word_data = (word_struct *)buffer;

				//printf("Got: '%s' | num = %i\n", word_data->word, word_data->num);

				//Check that recieved struct is correct and valid
				if(word_data->num == word_num){

					max_num = word_data->max_num;


					fputs(word_data->word, fp); //write word to the file

					if( word_num < max_num){
						fputs(" ", fp);  //if this word not last, append space ' ' after current written word
						//printf("It not end\n");
					}

					//iterates inly if current value recived packet is valid
					word_num++;
				}

				memset(buffer, 0, sizeof(buffer));
			}

			close(fp); //close the file


	    printf("TRANSFER COMPLETE \n", sent_recv_bytes);
			sleep(10);
    }
}
int
main(int argc, char **argv){
   char peer='x';
   while(1){
   printf("Run Peer as Server or as Client??\nwrite 'c' for client, 's' for the server\n");
      scanf("%c", &peer);
     if(peer == 'c'){
       tcp_client();
     }
   else if (peer = 's'){
       tcp_server();
     }
   }
   return 0;
}
