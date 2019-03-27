#include <stdio.h>
#include <stdlib.h>
#include "common.h"

test_struct_t client_data;

int main(){

  printf("Enter name: ?\n");
  scanf("%s", &client_data.name);
  printf("Enter age : ?\n");
  scanf("%s", &client_data.age);
  printf("Enter group: ?\n");
  scanf("%s", &client_data.group);

  printf("%s %u %s", client_data.name, client_data.age, client_data.group);
}
