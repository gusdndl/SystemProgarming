#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>

void print_size(off_t size) {
    const char *units[] = {"B", "KB", "MB", "GB"};
    int unit = 0;
    double s = (double)size;
    while (s >= 1024 && unit < 3) {
        s /= 1024;
        ++unit;
    }
    printf("%.2f %s\t", s, units[unit]);
}

void du(const char *name, int all) {
    DIR *dir;
    struct dirent *entry;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        
        struct stat st; 
        if (lstat(path, &st) == -1)
            continue;

        if (all || S_ISDIR(st.st_mode)) {
            print_size(st.st_blocks);
            printf("%s", path);
            
            if(S_ISDIR(st.st_mode)) 
                du(path, all);
         }
     }

     closedir(dir);
}

int main(int argc, char **argv) {
   int all = 0;

   for(int i=1; i<argc; ++i){
       if(strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "-ah") == 0 || strcmp(argv[i], "-ha") == 0)
           all = 1;
   }

   for(int i=1; i<argc; ++i){
       struct stat st; 
       if(lstat(argv[i], &st) != -1 && !S_ISDIR(st.st_mode)){
           print_size(st.st_blocks);
           printf("%s", argv[i]);
       }
       else if(strcmp(argv[i], "-a") != 0 && strcmp(argv[i], "-h") != 0 && strcmp(argv[i], "-ah") != 0 && strcmp(argv[i], "-ha") != 0){
           du(argv[i], all); // Start from given directory or file
       }
   }

   return EXIT_SUCCESS;
}
