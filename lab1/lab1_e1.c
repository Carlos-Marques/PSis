#include <stdio.h>

int strlength(char * string){
    int lenth = 0;
    
    while(*string != '\0') {
      lenth++;
      string++;
    }

    return lenth;
}

int main(int argc, char ** argv) {
    int counter = 1, lensum = 0;

    while(counter < argc) {
      lensum += strlength(argv[counter]);
      counter++;
    }

    printf("%d", lensum);
}
