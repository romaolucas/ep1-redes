#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char *minha_string = "\"string_entre_aspas\"";
    char string_limpa[100];
    printf("String original: %s\n", minha_string);
    strncpy(string_limpa, 1 + minha_string, strlen(minha_string) - 2);
    printf("String limpa: %s\n", string_limpa);
    return 0;
}
