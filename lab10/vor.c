/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <JlobCTEP> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Yuriy Sukhorukov
 * ----------------------------------------------------------------------------
 */

#include "array_list/alist.h"
#include <pthread.h>
#include <netdb.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <fcntl.h>

#define DEFAULT_PORT "8080"

#define NAME "-n"
#define ADDRESS "-a"
#define PORT "-p"
#define NODE_NAME "Tango\0"

#define REQUEST_CMD "-r"
#define SYN_CMD "-s"
#define CREATE_NEW_CMD "-mk"
#define FILENAME "-f"
#define CREATE_NEW 2
#define SYN 1
#define REQUEST 0
#define MAX_ATTEMPTS 3

#define SHARED_FOLDER "../shared_folder/"

#define SHTF(param) {printf("Incorrect parameters: %s\n", param);\
                     printf("Correct usage: %s\n %s\n %s\n", SYNTAX_CREATE_NEW, SYNTAX_SYN, SYNTAX_REQUEST);\
                     return NULL;};

#define SYNTAX_CREATE_NEW "-mk -n[name] in case of creating new network"
#define SYNTAX_SYN "-s -a[address] -p[port] -n[name] in case of syncing with existing network"
#define SYNTAX_REQUEST "-r -a[address] -p[port] -n[name] -f[filename] in case of syncing with existing network"

pthread_mutex_t lock_node_list, client, lock_file_list, server , lock_black_list, lock_current;

p_array_list node_list;
p_array_list black_list;
p_array_list current;
network_node * self;

void *connection_handler(void * data) {
    struct sockaddr_in client_addr;
    memcpy(&client_addr, data, sizeof(struct sockaddr_in));

    socklen_t addr_len;
    memcpy(&addr_len, data + sizeof(struct sockaddr_in), sizeof(socklen_t));

    int comm_socket;
    memcpy(&comm_socket, data + sizeof(struct sockaddr_in) + sizeof(socklen_t), sizeof(int));

    size_t hashed;
    memcpy(&hashed, data + sizeof(struct sockaddr_in) + sizeof(socklen_t) + sizeof(int), sizeof(size_t));

    printf("Connection accepted from client : %s:%u\n",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    unsigned int command = 100500;
    recvfrom(comm_socket, &command, sizeof(int), 0, (struct sockaddr *) &client_addr, &addr_len);
    command = htonl(command);
    printf("command %d\n", command);
    if (command == SYN) {

        char * buffer = malloc(MSG_LEN);
        memset(buffer, 0, MSG_LEN);
        recvfrom(comm_socket, buffer, MSG_LEN, 0,
                 (struct sockaddr *) &client_addr, &addr_len);

        network_node * nn = (network_node *)malloc(sizeof(network_node));
        memset(nn, 0, sizeof(network_node));
        split_msg(nn, buffer);
        free(buffer);

        pthread_mutex_lock(&lock_node_list);
        array_list_add(node_list, nn);
        pthread_mutex_unlock(&lock_node_list);
        int length = 0;
        recvfrom(comm_socket, &length, sizeof(int), 0,
                 (struct sockaddr *) &client_addr, &addr_len);
        length = ntohl((uint32_t) length);
        int i;
        for ( i = 0; i < length; i++) {
            network_node *nn2 = (network_node *) malloc(sizeof(network_node));
            memset(nn2, 0, sizeof(network_node));
            char *buffer2 = malloc(MSG_LEN);
            memset(buffer2, 0, MSG_LEN);
            recvfrom(comm_socket, buffer2, MSG_LEN, 0,
                     (struct sockaddr *) &client_addr, &addr_len);
            split_msg(nn2, buffer2);
            if (memcmp(nn2, self, NODE_LENGTH) != 0) {
                pthread_mutex_lock(&lock_node_list);
                array_list_add(node_list, nn2);
                pthread_mutex_unlock(&lock_node_list);
            }
            free(buffer2);
        }
        pthread_mutex_unlock(&lock_node_list);

    } else if (command == REQUEST) {

        char * filename = malloc(MSG_LEN);
        memset(filename, 0, MSG_LEN);
        printf("Receiving filename\n");
        recvfrom(comm_socket, filename, MSG_LEN, 0,
                 (struct sockaddr *) &client_addr, &addr_len);

        char filepath[MSG_LEN];
        memset(filepath, 0, MSG_LEN);
        strcpy(filepath, SHARED_FOLDER);
        strcat(filepath, filename);
        printf("filepath is %s\n", filepath);

        int word_count = 0;
        char ** parsed_file = parse_file(filepath, &word_count);
        printf("file was %d long\n", word_count);
        int word_count2 = htonl(word_count);
        sendto(comm_socket, &word_count2, sizeof(int), 0,
               (struct sockaddr *) &client_addr, sizeof(struct sockaddr));
        int i;
        for ( i = 0; i < word_count; i ++) {
            printf("word %s\n", parsed_file[i]);
            sendto(comm_socket, parsed_file[i], MSG_LEN, 0,
                   (struct sockaddr *) &client_addr, sizeof(struct sockaddr));
            usleep(50000);
        }
    }

    close(comm_socket);
    usleep(10000);
    pthread_mutex_lock(&lock_current);
    get_by_hash(current, hashed)->counter--;
    pthread_mutex_unlock(&lock_current);

    return NULL;
}


//done
void *tcp_server(void * nothing) {
    pthread_mutex_lock(&server);
    pthread_mutex_unlock(&server);
    int main_socket = 0;

    int comm_socket = 0;
    fd_set read_descriptors;
    struct sockaddr_in my_address,client_addr;

    my_address = *get_sockadrr(self);
    socklen_t addr_len = sizeof(struct sockaddr);

    main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    bind(main_socket, (struct sockaddr *) &my_address, sizeof(struct sockaddr));

    struct sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(main_socket, (struct sockaddr *)&sin, &len);
    listen(main_socket, 5);
    printf("Server is ready\n");
    while (1) {
        FD_ZERO(&read_descriptors);
        FD_SET(main_socket, &read_descriptors);
        select(main_socket + 1, &read_descriptors, NULL, NULL, NULL);

        if (FD_ISSET(main_socket, &read_descriptors)) {

            comm_socket = accept(main_socket, (struct sockaddr *) &client_addr, &addr_len);

            int flag = 1;

            network_node * nn = (network_node *)malloc(sizeof(network_node));
            sprintf(nn->node, ":%s:%d:", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            size_t hashed = hash_nn(nn);

            if (contains_by_hash(black_list, hashed) != 0) {
                flag = 0;
            } else if (contains_by_hash(current, hashed)) {
                if (get_by_hash(current, hashed)->counter >= MAX_ATTEMPTS) {
                    pthread_mutex_lock(&lock_black_list);
                    array_list_add(black_list, nn);
                    pthread_mutex_unlock(&lock_black_list);
                    printf("Node %s be black-listed\n", nn->node);
                    pthread_mutex_lock(&lock_current);
                    array_list_remove(current, nn);
                    pthread_mutex_unlock(&lock_current);
                    flag = 0;
                } else {
                    pthread_mutex_lock(&lock_current);
                    get_by_hash(current, hashed)->counter++;
                    pthread_mutex_unlock(&lock_current);
                }
            } else {
                pthread_mutex_lock(&lock_current);
                array_list_add(current, nn);
                get_by_hash(current, hashed)->counter++;
                pthread_mutex_unlock(&lock_current);
            }

            if (flag) {
                size_t size = sizeof(struct sockaddr_in) + sizeof(socklen_t) + sizeof(int) + sizeof(size_t);
                void *data = malloc(size);
                memset(data, 0, size);
                memcpy(data, &client_addr, sizeof(struct sockaddr_in));
                memcpy(data + sizeof(struct sockaddr_in), &addr_len, sizeof(socklen_t));
                memcpy(data + sizeof(struct sockaddr_in) + sizeof(socklen_t), &comm_socket, sizeof(int));
                memcpy(data + sizeof(struct sockaddr_in) + sizeof(socklen_t) + sizeof(int), &hashed, sizeof(size_t));

                pthread_t request;
                pthread_create(&request, NULL, connection_handler, data);
            }
        }
    }
}

int cl_parse(char *parameter, void **line, int line_size, size_t *index) {
    size_t i;
    for( i = 0; i < line_size; i++ ){
        if (strcmp(line[i], parameter) == 0 && i < line_size) {
            *index = i;
            return 0;
        }
    }
    return -1;
}

void * tcp_client(void * data) {
    pthread_mutex_lock(&client);
    pthread_mutex_unlock(&client);
    int cmd_len = 0;

    memcpy(&cmd_len, data, sizeof(int));
    void ** buffer = malloc(sizeof(void *) * cmd_len);
    memcpy(buffer, data + sizeof(int), sizeof(void *) * cmd_len);
    free(data);
    network_node * dest = (network_node *)malloc(sizeof(network_node));
    memset(dest, 0, sizeof(network_node));

    size_t index0 = 0;
    size_t index1 = 0;
    size_t index2 = 0;
    char node[NODE_LENGTH];
    char file_name[MSG_LEN];

    int command_counter;
    if (cl_parse(NAME, buffer, cmd_len, &index0) != -1 && index0 + 1 <= cmd_len) {

        char ip_address[15];
        int fd;
        struct ifreq ifr;
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        ifr.ifr_addr.sa_family = AF_INET;
        memcpy(ifr.ifr_name, "wifi0", IFNAMSIZ-1);

        ioctl(fd, SIOCGIFADDR, &ifr);
        close(fd);
        strcpy((char *) ip_address, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
        memset(node, 0, NODE_LENGTH);
        strcat(node, NODE_NAME);
        strcat(node, ":\0");
        strcat(node, ip_address);
        strcat(node, ":\0");
        strcat(node, DEFAULT_PORT);
        strcat(node, ":\0");

        strcpy(self->node, node);

        //handle make command
        if (cl_parse(CREATE_NEW_CMD, buffer, cmd_len, &index1) != -1) {
            command_counter = CREATE_NEW;
        //handle connect command
        } else {
            char address[20];
            char port[20];
            char *ptr;
            if (cl_parse(ADDRESS, buffer, cmd_len, &index1) != -1 && index1 + 1 <= cmd_len){
                if (cl_parse(PORT, buffer, cmd_len, &index2) != -1 && index2 + 1 <= cmd_len){
                    memset(node, 0, NODE_LENGTH);
                    strcat(node, buffer[index0 + 1]);
                    strcat(node, ":\0");
                    strcat(node, buffer[index1 + 1]);
                    strcat(node, ":\0");
                    strcat(node, buffer[index2 + 1]);
                    strcat(node, ":\0");

                    strcpy(dest->node, node);
                }else SHTF("port");
            } else SHTF("address");

            if (cl_parse(SYN_CMD, buffer, cmd_len, &index1) != -1 && index1 + 1 <= cmd_len)
                command_counter = SYN;
            else
                if (cl_parse(REQUEST_CMD, buffer, cmd_len, &index1) != -1 && index1 + 1 <= cmd_len) {
                    if (cl_parse(FILENAME, buffer, cmd_len, &index1) != -1 && index1 + 1 <= cmd_len) {
                        memset(file_name, 0, MSG_LEN);
                        strcpy(file_name, buffer[index1 + 1]);
                        command_counter = REQUEST;
                    }
                }
        }
    } else SHTF("name");

    pthread_mutex_unlock(&server);

    free(buffer);



    switch (command_counter) {
        case SYN: {
            int command = htonl(SYN);
            int zero = htonl(0);
            int main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            connect(main_socket, (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));
            sendto(main_socket, &command , sizeof(int), 0,
                   (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));

            sendto(main_socket, concat_msg(self), MSG_LEN, 0,
                   (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));

            sendto(main_socket, &zero, sizeof(int), 0,
                   (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));
            printf("done sync\n");
            break;
        }
        case REQUEST: {
            printf("execute order file transfer\n");

            int main_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            connect(main_socket, (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));

            int command = htonl(REQUEST);
            sendto(main_socket, &command, sizeof(int), 0,
                   (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));

            sendto(main_socket, file_name, MSG_LEN, 0,
                   (struct sockaddr *) get_sockadrr(dest), sizeof(struct sockaddr));

            int32_t len = 0;
            socklen_t addr_len = sizeof(struct sockaddr);
            recvfrom(main_socket, &len, sizeof(int32_t), 0,
                     (struct sockaddr *) get_sockadrr(dest), &addr_len);

            len = htonl(len);

            char file_content[MSG_LEN];
            memset(file_content, 0, MSG_LEN);
            int i;
            for ( i = 0; i < len; i++) {
                char word[MSG_LEN];
                memset(word, 0, MSG_LEN);
                recvfrom(main_socket, word, MSG_LEN, 0,
                         (struct sockaddr *) get_sockadrr(dest), &addr_len);
                strcat(file_content, word);
                strcat(file_content, " ");
                printf("word %s\n", word);
            }


            char filepath[FILENAME_LENGTH * 2];
            strcpy(filepath, SHARED_FOLDER);
            strcat(filepath, file_name);


            int fp = open(filepath, O_WRONLY | O_CREAT, S_IROTH | S_IWOTH);
            write(fp, file_content, MSG_LEN);
            close(fp);

            printf("done file transfer\n");

            close(main_socket);

            break;
        }        case CREATE_NEW:
            return NULL;

        default:
            return NULL;

    }
    return NULL;
}

void * syncher(void * nothing) {
    pthread_mutex_lock(&lock_node_list);
    while (1) {
        int is_shtf = 0;
        size_t iter = array_list_iter(node_list, &is_shtf);

        while (is_shtf >= 0) {
            socklen_t addr_len = sizeof(struct sockaddr);
            int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            struct sockaddr_in *dest = get_sockadrr(array_list_get(node_list, iter, &is_shtf));
            connect(sock, (struct sockaddr *) dest, sizeof(struct sockaddr));
            int command = htonl(1);
            sendto(sock, &command, sizeof(int), 0, (struct sockaddr *) &dest, sizeof(struct sockaddr));
            sendto(sock, concat_msg(self), MSG_LEN, 0, (struct sockaddr *) &dest, sizeof(struct sockaddr));
            int length = htonl((uint32_t) node_list->count);
            sendto(sock, &length, sizeof(int), 0, (struct sockaddr *) &dest, sizeof(struct sockaddr));
            if (node_list->count != 0) {
                int is_shtf2 = 0;
                size_t iter2 = array_list_iter(node_list, &is_shtf);
                while (is_shtf2 >= 0) {
                    char * buffer = malloc(MSG_LEN);
                    memset(buffer, 0, MSG_LEN);
                    strcpy(buffer, array_list_get(node_list, iter2, &is_shtf2)->node);
                    sendto(sock, buffer, MSG_LEN, 0, (struct sockaddr *) &dest, sizeof(struct sockaddr));
                    iter2 = array_list_next(node_list, iter2, &is_shtf2);
                }
            }
            iter = array_list_next(node_list, iter, &is_shtf);
        }
        pthread_mutex_unlock(&lock_node_list);
        sleep(10);
    }
}


void * file_daemon(void *nothing) {

    while (1) {
        DIR *d;
        struct dirent *dir;
        d = opendir(SHARED_FOLDER);
        pthread_mutex_lock(&lock_file_list);
        if (d) {
            while ((dir = readdir(d)) != NULL) {
                if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                    array_list_add_file(self, dir->d_name);
                    //printf("%s added\n", dir->d_name);
                }
            }
            closedir(d);
        }
        pthread_mutex_unlock(&lock_file_list);
        pthread_mutex_unlock(&client);
        sleep(10);
    }
}


int main(int argc, char **argv) {
    pthread_mutex_init(&lock_node_list, NULL);
    pthread_mutex_init(&client, NULL);
    pthread_mutex_init(&lock_file_list, NULL);
    pthread_mutex_init(&server, NULL);
    pthread_mutex_init(&lock_current, NULL);
    pthread_mutex_init(&lock_black_list, NULL);
    pthread_mutex_lock(&client);
    pthread_mutex_lock(&server);
    node_list = create_array_list();
    black_list = create_array_list();
    current = create_array_list();
    self = (network_node *)malloc(sizeof(network_node));
    memset(self, 0, sizeof(network_node));

    void * data = malloc(sizeof(int) + sizeof(void *) * argc);
    memcpy(data, &argc, sizeof(int));
    memcpy(data + sizeof(int), argv, sizeof(void *) * argc);


    pthread_t sync, tcp_serv, tcp_cli, daemon;
    pthread_create(&tcp_cli, NULL, tcp_client, data);
    pthread_create(&tcp_serv, NULL, tcp_server, NULL);
    pthread_create(&daemon, NULL, file_daemon, NULL);
    pthread_create(&sync, NULL, syncher, NULL);
    pthread_join(sync, NULL);
    pthread_join(tcp_serv, NULL);
    pthread_join(tcp_cli, NULL);
    pthread_join(daemon, NULL);
}
