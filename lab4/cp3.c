#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1024

void copy_file(const char *src, const char *dest) {
    int srcFile = open(src, O_RDONLY);
    if (srcFile == -1) {
        perror("Unable to open source file");
        exit(EXIT_FAILURE);
    }

    int destFile = open(dest, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (destFile == -1) {
        perror("Unable to open destination file");
        close(srcFile);
        exit(EXIT_FAILURE);
    }

    char buf[BUF_SIZE];
    ssize_t bytesRead;
    
    while ((bytesRead = read(srcFile, buf, BUF_SIZE)) > 0) {
        if(write(destFile, buf, bytesRead) != bytesRead){
            perror("Writing error occurred");
            close(srcFile);
            close(destFile);
            exit(EXIT_FAILURE);            
        }
    }

   if(bytesRead == -1){
       perror("Reading error occurred");
       close(srcFile);
       close(destFile);
       exit(EXIT_FAILURE); 
   } 

   //printf("Files copied successfully.\n");

   close(srcFile);
   close(destFile);   
}

int main(int argc, char **argv) {
   if(argc != 3){
       printf("Usage: ./prog sourcefile destinationfile");
       return 1;
   }
   
   copy_file(argv[1], argv[2]);

}
