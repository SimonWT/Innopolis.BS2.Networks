
#include <stdio.h>
#include "array_list/alist.h"
#include <netdb.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ffi.h>
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


int main() {
    size_t lol = 0;
    network_node * nn2 = (network_node *)malloc(sizeof(network_node));
    char str[FILE_LIST_LENGTH];
    memset(str, 0, FILE_LIST_LENGTH);
    strcpy(str, "Alpha:127.0.0.1:8080:sfgfsdg,gdfgdg,qqwd,sfdsfsd\0");
    split_msg(nn2, str);
    array_list_add_file(nn2, "lolkekmem");

    struct sockaddr_in * kek;
    kek = get_sockadrr(nn2);

    char str2[FILE_LIST_LENGTH];
    memset(str2, 0, FILE_LIST_LENGTH);
    strcpy(str2, "Beta:127.0.0.1:8080:afdssdf,ertd,sdfgreg,sesd\0");
    network_node * nn3 = (network_node *)malloc(sizeof(network_node));
    split_msg(nn3, str2);

    char str3[FILE_LIST_LENGTH];
    memset(str3, 0, FILE_LIST_LENGTH);
    strcpy(str3, "Gamma:127.0.0.1:8080:skfldgh,eoriut,sdfsd\0");
    network_node * nn4 = (network_node *)malloc(sizeof(network_node));
    split_msg(nn4, str3);

    struct sockaddr_in * sa = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    memset(sa, 0, sizeof(struct sockaddr_in));

    sa = get_sockadrr(nn3);
    printf("%zu \n%zu", hash_nn(nn3), hash_sockadrr(sa));

    p_array_list alist = create_array_list();

    array_list_add(alist, nn2);
    array_list_add(alist, nn3);
    array_list_add(alist, nn4);

    printf("size: %ld, count: %ld\n", alist->size, alist->count);
    int is_shtf = 1;
    size_t length;


    size_t iter = array_list_iter(alist, &is_shtf);
    while(is_shtf >= 0) {
        printf("%zu \n", (array_list_get(alist, iter, &is_shtf))->counter);
        iter = array_list_next(alist, iter, &is_shtf);
    }
    printf("kek\n");

    delete_array_list(alist);

    return 0;
}
