#include <stdio.h>
#include <stdlib.h>

void read_bin(char* target_file) {
    unsigned char buffer[500];
    FILE *ptr;
    ptr = fopen(target_file, "rb");
    fread(buffer, sizeof(buffer),1,ptr);
    
    for(int i = 0; i<1000; i++) {
        printf("%u ", buffer[i]);
    }
}

int main(int argc, char* argv[]) {
    read_bin(argv[1]);
    return 0;
}