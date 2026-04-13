#include "string.h"

int my_strlen(char* str) {
    int i = 0;
    while (str[i] != '\0') i++;
    return i;
}

void my_strcpy(char* dest, const char* src) {
    int i = 0;
    while (src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void my_strcat(char* dest, const char* src) {
    int i = my_strlen(dest);
    int j = 0;
    while (src[j] != '\0') {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
}
