#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void print_size(off_t blocks, int human_readable) {
    const char *units[] = {"K", "M", "G", "T"};
    int i = 0;
    double s = (double)blocks / 2; // convert blocks to KB assuming block size is 512 bytes

    if (human_readable) {
        while (s >= 1024 && i < 3) {
            s /= 1024;
            i++;
        }
        printf("%.1f%s\t", s, units[i]);
    } else {
        printf("%lld\t", (long long)s);
    }
}

off_t du(const char *path, int all, int human_readable) {
    DIR *dirp;
    struct dirent *dp;

    if ((dirp = opendir(path)) == NULL)
        return 0; // If it's a file or not accessible return the size as zero

    off_t total_size = 8; // Initialize the total size as zero or the default size for directories

    while ((dp = readdir(dirp)) != NULL) {

        struct stat st;
        char filepath[1024];

        snprintf(filepath, sizeof(filepath), "%s/%s", path, dp->d_name);

         if(stat(filepath, &st)==-1 || dp->d_name[0] == '.'){
            continue;   
         }

         if(dp->d_type == DT_DIR && strcmp(dp->d_name,".")!=0 && strcmp(dp->d_name,"..")!=0){
             off_t dir_size = du(filepath , all , human_readable); 
             total_size += dir_size;

             // Print directories always
             print_size(dir_size ,human_readable); 
             printf("%s\n", filepath);
          }

          else{
           total_size += st.st_blocks;

           // Print both directories and files when -a option is given
           if(all){
               print_size(st.st_blocks ,human_readable); 
               printf("%s\n", filepath);
           }
          }       
      }

      closedir(dirp);

      return total_size;
}


int main(int argc,char* argv[]) {

   int all=0,human_readable=0;
   char* dir_paths[argc];
   int dir_count = 0;

   for(int i=1;i<argc;i++){
      if(strcmp(argv[i],"-a")==0 || strcmp(argv[i], "-ah") == 0 || strcmp(argv[i], "-ha") == 0){
         all=1;
      } 
      
      if(strcmp(argv[i],"-h")==0 || strcmp(argv[i], "-ah") == 0 || strcmp(argv[i], "-ha") == 0){
         human_readable=1;   
      }

      // If it's not an option, it's a directory or file
      if(argv[i][0] != '-'){
         dir_paths[dir_count++] = argv[i];
      }
   }

   // If no directory is specified, use the current directory.
   if (dir_count == 0) {
       dir_paths[dir_count++] = ".";
   }

   for (int i = 0; i < dir_count; i++) {
       off_t total_size = du(dir_paths[i] ,all,human_readable);
       print_size(total_size, human_readable);
       printf("%s\n", dir_paths[i]);
   }

   return 0;  
}
