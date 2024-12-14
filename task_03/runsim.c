#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 100

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <maximal_process_number>\n", argv[0]);
    return 1;
  }

  int max_processes;
  max_processes = atoi(argv[1]);

  if (max_processes <= 0 || max_processes > INT_MAX) {
    fprintf(stderr, "Invalid number of processes\n");
    return 1;
  }

  int process_count = 0;
  char input[1024]; // Adjust buffer size as needed

  while (fgets(input, sizeof(input), stdin) != NULL) {
    // Remove trailing newline
    input[strcspn(input, "\n")] = '\0';

    // Find finished process
    int status;
    while (process_count) {
      pid_t finished_pid = waitpid(-1, &status, WNOHANG);
      if (finished_pid > 0)
        process_count--;
      else
        break;
    }

    // Skip command if too many processes are running
    if (process_count >= max_processes) {
      fprintf(
          stderr,
          "Too many processes running. Please wait for some to finish...\n");
      continue;
    }

    char *command = strtok(input, " ");

    // Prepare arguments for execvp
    char *args[MAX_ARGS];
    args[0] = command; // First argument is the command itself
    int i = 1;

    // Get additional arguments if any
    char *arg;
    while ((arg = strtok(NULL, " \n")) != NULL && i < MAX_ARGS - 1)
      args[i++] = arg;
    args[i] = NULL; // Null-terminate the argument list

    // Create new process
    pid_t pid = fork();
    if (pid < 0) {
      perror("Fork failed");
      return 1;
    } else if (pid == 0) { // Child process
      if (execvp(command, args) == -1) {
        fprintf(stderr, "Error executing command: %s\n", command);
        exit(1);
      }
    } else { // Parent process
      ++process_count;
    }
  }

  // Wait for all remaining processes to finish
  while (wait(NULL) > 0)
    ;

  return 0;
}