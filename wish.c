#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

char** path;

void error(bool is_child) {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
  if (is_child) exit(1);
}

void write_path(char** array, int from, int to) {
  path = (char**)realloc(path, (to - from + 2) * sizeof(char*));
  int j;
  int i = 0;
  for (j = from; j <= to; j++) {
    path[i] = array[j];
    i++;
  }
  path[i] = NULL;
}

void execute(char* command) {
  // Parse command
  char* file = strchr(command, '>');
  int redirect = false;
  if (file != NULL) {
    redirect = true;
    *file = '\0';
    file++;
    while (*file == ' ' || *file == '\t') {
      file++;
    }
    char* _f = file;
    while (*_f != '\n') _f++;
    *_f = '\0';
  }

  char* args[32];
  char* arg = strtok(command, " \t\n");
  int i = 0;
  while (arg != NULL) {
    args[i++] = arg;
    arg = strtok(NULL, " \t\n");
  }
  args[i] = NULL;

  // Built-in commands
  if (strcmp(args[0], "exit") == 0) {
    if (i > 1) {
      error(0);
    } else {
      exit(0);
    }
    return;
  } else if (strcmp(args[0], "cd") == 0) {
    if (i == 2) {
      if (chdir(args[1]) == -1) {
        error(0);
      }
    } else {
      error(0);
    }
    return;
  } else if (strcmp(args[0], "path") == 0) {
    write_path(args, 1, i);
    return;
  }

  int id = fork();
  if (id < 0) {
    exit(1);
  } else if (id == 0) {
    char** p = path;
    char* full_path;
    while (*p != NULL) {
      full_path = strdup(*p);
      strcat(full_path, "/");
      strcat(full_path, args[0]);
      if (access(full_path, X_OK) == 0) {

        if (redirect) {
          close(STDOUT_FILENO);
          open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
        }
        
        execv(full_path, args);
      }
      p++;
    }

    if (redirect) {
      close(STDERR_FILENO);
      open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
    }

    error(1);
  } else {
    int child = wait(NULL);
  }
}

int main(int argc, const char* argv[]) {
  char* input;
  size_t bufsize = 4096;
  char* init_path[] = {"/bin", NULL};
  write_path(init_path, 0, 1);
  while (true) {
    printf("wish> ");
    input = (char*)malloc(bufsize * sizeof(char));
    getline(&input, &bufsize, stdin);
    char* cmd;
    char* command; 
    while ((cmd = strchr(input, '&')) != NULL) {
      *cmd = '\0';
      command = strdup(input);
      execute(command);
      free(command);
      cmd++;
      input = cmd;
    }
    command = strdup(input);
    execute(command);
    free(command);
  }
  free(input);
  return 0;
}