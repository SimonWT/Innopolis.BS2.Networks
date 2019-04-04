/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <beluckydaf@gmail.com> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Vladislav Smirnov
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include "alist.h"

p_array_list create_array_list() {
    network_node* array = (network_node *)malloc(sizeof(network_node) * NODES_INIT_SIZE);
    p_array_list list = (p_array_list)malloc(sizeof(array_list));
    memset(array, 0, sizeof(network_node) * NODES_INIT_SIZE);
    list->size = NODES_INIT_SIZE;
    list->count = 0;
    list->nodes = array;
    return list;
}

char ** parse_files(network_node * nn, size_t * count, size_t * last_pos) {
    size_t i = 0;
    size_t file_count = 0;
    while (i < FILE_LIST_LENGTH && nn->files[i] != '\0') {
        if (nn->files[i] == ',')
            file_count++;
        i++;
    }
    file_count++;
    char ** array = (char **)malloc(file_count * sizeof(void *));
    for (int x = 0; x < file_count; x++) {
        array[x] = malloc(FILENAME_LENGTH);
        memset(array[x], 0, FILENAME_LENGTH);
    }

    char buffer[FILENAME_LENGTH];
    memset(buffer, 0, FILENAME_LENGTH);
    size_t buffer_count = 0;
    size_t array_count = 0;
    i = 0;
    while (i < FILE_LIST_LENGTH && nn->files[i] != '\0') {
        if (nn->files[i] == ',') {
            strcpy(array[array_count], buffer);
            memset(buffer, 0, FILENAME_LENGTH);
            buffer_count = 0;
            array_count++;
        } else {
            buffer[buffer_count] = nn->files[i];
            buffer_count++;
        }
        i++;
    }
    strcpy(array[array_count], buffer);
    memset(buffer, 0, FILENAME_LENGTH);
    *count = file_count;
    *last_pos = i;
    return array;
}

struct sockaddr_in * get_sockadrr(network_node *nn) {
    struct sockaddr_in * address;
    address = malloc(sizeof(struct sockaddr_in));
    memset(address, 0, sizeof(struct sockaddr_in));

    size_t size = 0;
    char ** parsed = parse_nodes(nn, &size);

    address->sin_family = AF_INET;
    inet_pton(AF_INET, parsed[1], &address->sin_addr);
    address->sin_port  = htons((uint16_t)atoi(parsed[2]));
    return address;
}


char ** parse_nodes(network_node * nn, size_t * last_pos) {
    char buffer[FILENAME_LENGTH];
    memset(buffer, 0, FILENAME_LENGTH);
    size_t buffer_count = 0;
    size_t array_count = 0;
    size_t i = 0;

    char ** array = (char **)malloc(3 * sizeof(void *));
    for (int x = 0; x < 3; x++) {
        array[x] = malloc(FILENAME_LENGTH);
        memset(array[x], 0, FILENAME_LENGTH);
    }

    while (i < FILE_LIST_LENGTH && nn->node[i] != '\0') {
        if (nn->node[i] == ':') {
            strcpy(array[array_count], buffer);
            memset(buffer, 0, FILENAME_LENGTH);
            buffer_count = 0;
            array_count++;
        } else {
            buffer[buffer_count] = nn->node[i];
            buffer_count++;
        }
        i++;
    }
    *last_pos = i;
    return array;
}

void split_msg(network_node * nn, char * msg) {
    size_t i = 0;
    size_t splitter = 0;
    while (i < FILE_LIST_LENGTH && msg[i] != '\0') {
        if (msg[i] == ':')
            splitter = i;
        i++;
    }
    memcpy(nn->node, msg, splitter + 1);
    memcpy(nn->files, msg + splitter + 1, FILE_LIST_LENGTH);
}

char * concat_msg(network_node * nn) {
    char * buffer = malloc(MSG_LEN);
    memset(buffer, 0, MSG_LEN);
    strcat(buffer, nn->node);
    strcat(buffer, nn->files);
    return buffer;
}

void delete_array_list(p_array_list list) {
    free(list->nodes);
    free(list);
}


size_t enlarge_array_list(p_array_list list) {
    size_t size = list->size;
    size_t new_size = size * 2;
    network_node* array = (network_node *)malloc(sizeof(network_node) * new_size);
    memset(array, 0, sizeof(network_node) * new_size);
    memcpy(array, list->nodes, sizeof(network_node) * size);
    free(list->nodes);
    list->nodes = array;
    list->size = new_size;
    return new_size;
}

size_t array_list_add(p_array_list list, network_node* item) {
    for (size_t i = 0; i < list->size; i++) {
        if (memcmp(&list->nodes[i], &(network_node){0}, sizeof(network_node)) == 0) {
            memcpy(&list->nodes[i], item, sizeof(network_node));
            list->count++;
            return i;
        }
        if (strcmp(list->nodes[i].node, item->node) == 0 && strcmp(item->files, "") != 0) {
            memcpy(&list->nodes[i], item, sizeof(network_node));
            return i;
        }
    }
    size_t index = (enlarge_array_list(list) / 2);
    memcpy(&list->nodes[index], item, sizeof(network_node));;
    list->count++;
    return index;
}

size_t array_list_remove(p_array_list list, network_node * item) {
    for (size_t i = 0; i < list->size; i++) {
        if (memcmp(&list->nodes[i], item, NODE_LENGTH) == 0) {
            memset(&list->nodes[i], 0, sizeof(network_node));
            list->count++;
            return i;
        }
    }
    return 0;
}

void array_list_add_file(network_node* item, char * file) {
    size_t file_count = 0;
    size_t last_pos = 0;
    char ** existing_files = parse_files(item, &file_count, &last_pos);
    for (size_t i = 0; i < file_count; i ++) {
        if (strcmp(existing_files[i], file) == 0)
            return;
    }
    size_t offset = 0;
    if (last_pos != 0) {
        item->files[last_pos] = ',';
        offset = 1;
    }
    strcpy(&item->files[last_pos + offset], file);
}

void array_list_clear_files(network_node *item) {
    memset(item->files, 0, FILE_LIST_LENGTH);
}


size_t array_list_iter(p_array_list list, int * is_error) {
    if (list->count == 0) {
        *is_error = -1;
        return 0;
    }
    for (size_t i = 0; i < list->count; i++) {
        if (memcmp(&list->nodes[i], &(network_node){0}, sizeof(network_node)) != 0) return i;
    }
    *is_error = -1;
    return 0;
}

size_t array_list_next(p_array_list list, size_t index, int * is_error) {
    for (size_t i = index + 1; i < list->size; i++) {
        if (memcmp(&list->nodes[i], &(network_node){0}, sizeof(network_node)) != 0) return (size_t) i;
    }
    *is_error = -1;
    return 0;
}

network_node* array_list_get(p_array_list list, size_t index, int * is_error) {
    if (index > list->size) {
        *is_error = -1;
        return NULL;
    }
    return &list->nodes[index];
}

size_t hash_string(char * str) {
    unsigned long hash = 5381;
    size_t c;
    while (c = *str++)
        hash = ((hash << 5) + hash) + c;
    return hash;
}

size_t hash_nn(network_node *nn) {
    int i = 0;
    for (i = 0; nn->node[i] != ':'; i++);
    int j = i + 1;
    for (j; nn->node[j] != ':'; j++);
    char buffer[NODE_LENGTH];
    memset(buffer, 0, NODE_LENGTH);
    memcpy(buffer, nn->node + i, j - 1 + 1);
    return hash_string(buffer);
}

int contains_by_hash(p_array_list list, size_t hashed) {
    for (int i = 0; i < list->count; i++) {
        if (hash_nn(&list->nodes[i]) == hashed)
            return 1;
    }
    return 0;
}

network_node * get_by_hash(p_array_list list, size_t hashed) {
    for (int i = 0; i < list->count; i++) {
        if (hash_nn(&list->nodes[i]) == hashed)
            return &list->nodes[i];
    }
    return 0;
}

char ** parse_file(char * path, int * word_count) {

    FILE *fp;
    char ch;
    long size = 0;
    printf("filepath is %s\n", path);
    fp = fopen(path, "r");
    fseek(fp, 0, 2);
    size = ftell(fp);
    fclose(fp);

    printf("Size %li\n", size);

    int file = open(path, O_RDONLY);
    char * buffer = malloc((size_t) size);
    memset(buffer, 0, size);
    read(file, buffer, size);
    *word_count = 1;
    for (int i = 0; i < size; i++) {
        if (buffer[i] == ' ')
            *word_count = *word_count + 1;
    }

    char ** parsed_file = (char **)malloc(*word_count * sizeof(void *));
    for (int i = 0; i < *word_count; i++) {
        parsed_file[i] = malloc(MSG_LEN);
        memset(parsed_file[i], 0, MSG_LEN);
    }

    char word_buffer[MSG_LEN];
    memset(word_buffer, 0, MSG_LEN);
    int buffer_iter = 0;
    int parsed_counter = 0;
    for(int i = 0; i < size; i++) {
        if (buffer[i] == *" "){
            strcpy(parsed_file[parsed_counter], word_buffer);
            parsed_counter++;
            memset(word_buffer, 0, MSG_LEN);
            buffer_iter = 0;
        } else {
            word_buffer[buffer_iter] = buffer[i];
            buffer_iter ++;
        }
    }
    strcpy(parsed_file[parsed_counter], word_buffer);
    return parsed_file;
}
