#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"

#define MYPORT "4950" // the port users will be connecting to

#define MAXBUFLEN 200

/*Server process is running on this port no. Client has to send data to this port no*/

test_struct_t test_struct;
result_struct_t res_struct;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
								if (sa->sa_family == AF_INET) {
																return &(((struct sockaddr_in*)sa)->sin_addr);
								}

								return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void
setup_udp_server_communication(){

								//**  UDP Listener part  **///
								int sockfd;
								struct addrinfo hints, *servinfo, *p;
								int rv;
								int numbytes;
								struct sockaddr_storage their_addr;
								char buf[MAXBUFLEN];
								socklen_t addr_len;
								char s[INET6_ADDRSTRLEN];

								memset(&hints, 0, sizeof hints);

								hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
								hints.ai_socktype = SOCK_DGRAM;
								hints.ai_flags = AI_PASSIVE; // use my IP

								if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) {
																fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
																return 1;
								}

								//CREATE SOCKET
								// loop through all the results and bind to the first we can
								for(p = servinfo; p != NULL; p = p->ai_next) {
																if ((sockfd = socket(p->ai_family, p->ai_socktype,
																																					p->ai_protocol)) == -1) {
																								perror("socket creation failed");
																								continue;
																}

																//BIND SOCKET
																if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
																								close(sockfd);
																								perror("socket bind failed");
																								continue;
																}

																break;
								}

								freeaddrinfo(servinfo);
								//** END UDP Listener part **//

								while(1) {



																printf("Server ready to service client msgs: waiting to recvfrom...\n");

																addr_len = sizeof their_addr;

																if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1, 0,
																																									(struct sockaddr *)&their_addr, &addr_len)) == -1) {
																								perror("error recvfrom");
																								exit(1);
																}

																printf("server: got packet from %s\n",
																							inet_ntop(their_addr.ss_family,
																																	get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));

																printf("server: packet is %d bytes long\n", numbytes);

																// buf[numbytes] = '\0';
																// printf("listener: packet contains \"%s\"\n", buf);

																test_struct_t *client_data = (test_struct_t *)buf;


																result_struct_t result;

																char tmp[160];
																strcpy(tmp, client_data->name);
																strcat(tmp, " ");
																strcat(tmp, client_data->age);
																strcat(tmp, " ");
																strcat(tmp, client_data->group);
																strcat(tmp, " ~ PIDOR");

																strcpy(result.c, tmp);

																/* Server replying back to client now*/
																int sent_recv_bytes = sendto(sockfd, (char *)&result, sizeof(result_struct_t), 0,
																																									(struct sockaddr *)&their_addr, sizeof(struct sockaddr));

																printf("Server sent %d bytes in reply to client\n", sent_recv_bytes);
								}

}

int
main(int argc, char **argv){

								setup_udp_server_communication();
								return 0;
}
