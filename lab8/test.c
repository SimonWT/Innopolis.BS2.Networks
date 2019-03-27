#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include "common.h"

int main(){
  FILE* fp =fopen("test.txt", "w");
  word_struct word_data;
  strcpy(word_data.word, "keksik");
  word_data.max_num = 1;
  word_data.num =1;

  while(1){
  fputs(word_data.word, fp);
  if(word_data.max_num > word_data.num)
    fputs(" ", fp);
  else break;
  }
  return 0;
}
