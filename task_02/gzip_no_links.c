#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <wait.h>

void backup_file(const char *src_file, const char *dst_file) {
  // Open the source file for reading
  int src_fd = open(src_file, O_RDONLY);
  if (src_fd < 0) {
    perror("Error opening source file");
    return;
  }

  // Open/create the destination file for writing
  int dst_fd = open(dst_file, O_WRONLY | O_CREAT | O_TRUNC,
                    0644); // rw for owner, r for group and otherss
  if (dst_fd < 0) {
    perror("Error creating destination file");
    close(src_fd);
    return;
  }

  // Copy the content
  char buffer[4096];
  ssize_t bytes;
  while ((bytes = read(src_fd, buffer, sizeof(buffer))) > 0) {
    write(dst_fd, buffer, bytes);
  }

  //  Close the files
  close(src_fd);
  close(dst_fd);

  // Fork a child process
  pid_t pid = fork();

  if (pid == 0) { // Child process
    // Compress the file
    if (execlp("gzip", "gzip", dst_file, NULL) == -1) {
      perror("Error executing command: 'gzip'\n");
      return;
    }
  } else if (pid > 0) { // Parent process
    // Wait for the child process to finish
    waitpid(pid, NULL, 0);
  } else { // Error during fork
    fprintf(stderr, "Error forking process: %s\n", strerror(errno));
    return;
  }
}

void backup_directory(const char *src_dir, const char *dst_dir) {
  // Check if the source directory exists
  struct stat st;
  if (stat(src_dir, &st) != 0 || !S_ISDIR(st.st_mode)) {
    fprintf(stderr,
            "Source directory '%s' does not exist or is not a directory.\n",
            src_dir);
    return;
  }

  //  Open the source directory
  DIR *dir = opendir(src_dir);
  if (dir == NULL) {
    perror("Error opening directory");
    return;
  }

  //  Open/create the destination directory
  if (stat(dst_dir, &st) != 0) {
    // Destination directory does not exist, create it
    if (mkdir(dst_dir, 0755) != 0) {
      perror("Error creating destination directory");
      return;
    }
  }

  // Read entries from the directory
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Ignore special directories . and ..
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    // Construct the full path for the source
    char src_path[512];
    snprintf(src_path, sizeof(src_path), "%s/%s", src_dir, entry->d_name);

    // Construct the full path for the destination
    char dst_path[512];
    snprintf(dst_path, sizeof(dst_path), "%s/%s", dst_dir, entry->d_name);

    struct stat src_stat;
    lstat(src_path, &src_stat);

    // Check if the entry is a symbolic link
    if (S_ISLNK(src_stat.st_mode)) {
      continue; // Ignore symbolic links
    } else if (S_ISDIR(src_stat.st_mode)) {
      // Recursive call for subdirectories
      mkdir(dst_path, 0755);
      backup_directory(src_path, dst_path);
    } else if (S_ISREG(src_stat.st_mode)) {
      // Check if the file exists in the destination directory
      char dst_gz_path[515];
      snprintf(dst_gz_path, sizeof(dst_gz_path), "%s.gz", dst_path);
      struct stat dst_gz_stat;
      if (stat(dst_gz_path, &dst_gz_stat) == -1) {
        // File was not backed up
        backup_file(src_path, dst_path);
      } else {
        // Check last back up time
        if (src_stat.st_mtime > dst_gz_stat.st_mtime) {
          // File has been modified
          remove(dst_gz_path);
          backup_file(src_path, dst_path);
        }
      }
    }
  }

  closedir(dir);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    fprintf(stderr, "Usage: %s <source directory> <destination directory>\n",
            argv[0]);
    return 1;
  }

  backup_directory(argv[1], argv[2]);
  return 0;
}