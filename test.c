
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
static_assert (sizeof(size_t)==8, "we assume that its running on 64bit");

int isValidCharacter(char * c) {
  printf("%c\n",c[0]);
  return 1;
}


int main(){
    char word[20] = "this is a test";
    char *ptr = word;
    int testing = isValidCharacter(ptr);
    return testing;
}