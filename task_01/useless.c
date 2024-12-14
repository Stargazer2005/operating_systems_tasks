#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define MAX_ARGS 100
#define MAX_LINES 100

typedef struct {
  int delay;
  char command[1024];
} Command;

int compare(const void *a, const void *b) {
  return ((Command *)a)->delay - ((Command *)b)->delay;
}

int main(int argc, char *argv[]) {
  // Check if the number of arguments is correct
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  // Check if the file exists
  char *filename = argv[1];
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
    return 1;
  }

  Command commands[MAX_LINES];
  int command_count = 0;

  // Read the file and store commands
  char line[1024];
  while (fgets(line, sizeof(line), file) != NULL) {
    char *delay_str = strtok(line, " ");
    char *command = strtok(NULL, "\n");

    // Check if the line is in the correct format
    if (delay_str == NULL || command == NULL) {
      fprintf(stderr, "Invalid line in file: %s\n", line);
      continue;
    }

    // Store the delay and command
    commands[command_count].delay = atoi(delay_str);
    strncpy(commands[command_count].command, command,
            sizeof(commands[command_count].command));
    command_count++;

    if (command_count == MAX_LINES) {
      fprintf(stderr, "Exceeded maximum number of commands\n");
      break;
    }
  }

  fclose(file);

  // Sort commands based on delay
  qsort(commands, command_count, sizeof(Command), compare);

  int i;
  // Convert absolute delay to relative
  for (i = command_count - 1; i > 0; i--) {
    commands[i].delay = commands[i].delay - commands[i - 1].delay;
  }

  // Execute commands in sorted order
  for (i = 0; i < command_count; i++) {
    sleep(commands[i].delay);

    // Prepare arguments for execvpf
    char *args[MAX_ARGS];
    args[0] = strtok(commands[i].command,
                     " \n"); // First argument is the command itself

    int j = 1;
    // Get additional arguments if any
    char *arg;
    while ((arg = strtok(NULL, " \n")) != NULL && j < MAX_ARGS - 1) {
      args[j++] = arg;
    }
    args[j] = NULL; // Null-terminate the argument list

    // Fork a child process
    pid_t pid = fork();

    if (pid == 0) { // Child process
      // Execute the command using execvp
      if (execvp(commands[i].command, args) == -1) {
        fprintf(stderr, "Error executing command: %s\n", commands[i].command);
        return 1;
      }
    } else if (pid > 0) { // Parent process
      // Wait for the child process to finish
      waitpid(pid, NULL, 0);
    } else { // Error during fork
      fprintf(stderr, "Error forking process: %s\n", strerror(errno));
      return 1;
    }
  }

  return 0;
}