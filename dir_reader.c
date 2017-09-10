#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>

int main() {
    char directory[50];
    DIR *new_dir, *cur_dir;
    char new[100]; 
    char cur[100];
    struct dirent *entry;
    int recent = 0;
    int exists = 0;
    strcpy(directory, "romao@test/Maildir");
    printf("directory: %s\n", directory);
    strcpy(new, directory);
    strcpy(cur, directory);
    strcat(new, "/new");
    strcat(cur, "/cur");
    printf("new: %s, cur: %s\n", new, cur);
    new_dir = opendir(new);
    printf("vou contar o new\n");
    while ((entry = readdir(new_dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            recent++;
        }
    }
    closedir(new_dir);
    printf("parei de contar o new\n");
    exists += recent;
    cur_dir = opendir(cur);
    printf("vou começar a contar o cur\n");
    while ((entry = readdir(cur_dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            exists++;
        }
    }
    closedir(cur_dir);
    printf("parei de contar o cur\n");
    printf("%d recent, %d exists\n", recent, exists);
    return 0;
}
