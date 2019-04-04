/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <beluckydaf@gmail.com> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Vladislav Smirnov
 * ----------------------------------------------------------------------------
 */

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#define NODE_LENGTH 256
#define FILE_LIST_LENGTH 760
#define MSG_LEN NODE_LENGTH + FILE_LIST_LENGTH + sizeof(size_t)
#define FILENAME_LENGTH 64
#define NODES_INIT_SIZE 4


typedef struct {
    char node[NODE_LENGTH];
    char files[FILE_LIST_LENGTH];
    size_t counter;
} network_node;

typedef struct {
    size_t size;
    size_t count;
    network_node* nodes;
} array_list;

typedef array_list* p_array_list;

p_array_list create_array_list();
void delete_array_list(p_array_list list);
size_t enlarge_array_list(p_array_list list);
size_t array_list_add(p_array_list list, network_node* item);
size_t array_list_remove(p_array_list list, network_node * item);
size_t array_list_iter(p_array_list list, int * is_error);
size_t array_list_next(p_array_list list, size_t index, int * is_error);
network_node* array_list_get(p_array_list list, size_t index, int * is_error);
void split_msg(network_node * nn, char * msg);
char * concat_msg(network_node * nn);
int contains_by_hash(p_array_list list, size_t hashed);
void array_list_add_file(network_node* item, char * file);
char ** parse_nodes(network_node * nn, size_t * last_pos);
char ** parse_files(network_node * nn, size_t * count, size_t * last_pos);
struct sockaddr_in * get_sockadrr(network_node *nn);
network_node * get_by_hash(p_array_list list, size_t hashed);
char ** parse_file(char * path, int * word_count);
size_t hash_nn(network_node *nn);
size_t hash_string(char * str);