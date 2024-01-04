#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

char** path;
int pathlen;

void error(bool is_child) {
  char error_message[30] = "An error has occurred\n";
  write(STDERR_FILENO, error_message, strlen(error_message));
  if (is_child) exit(1);
}

void write_path(char** array, int length) {
  // Free the memory of the old strings in the path array
  for (int i = 0; i < pathlen; i++) {
    free(path[i]);
  }
  pathlen = length;
  size_t bytes = (length) * sizeof(char*);
  path = (char**)realloc(path, bytes);
  for (int i = 0; i < length; i++) {
    path[i] = strdup(array[i]);
  }
}

void print_path() {
  printf("%d\n", pathlen);
  for (int i = 0; i < pathlen; i++) {
    printf("%s\n", path[i]);
  }
}

void execute(char* input) {
  char* saveptr;
  char* command = strtok_r(input, "&", &saveptr);
  int forks[10];
  int pi = 0;
  
  while ((command != NULL)) {
    // Parse command
    char* file = strchr(command, '>');
    bool redirect = false;
    if (file != NULL) {
      redirect = true;
      *file = '\0';
      file++;

      while (*file == ' ' || *file == '\t') {
        file++;
      }

      // > file   \0 (ok)
      // > file   \n\0 (ok)
      // > file1  file2  \n\0 (not ok)
      char* _f = strtok(file, " \t\n");
      if ((_f = strtok(NULL, " \t\n")) != NULL) {
        while (*_f == ' ' || *_f == '\t' || *_f == '\n') {
          _f++;
        }
        if (*_f != '\0') {
          error(0);
          return;
        }
      }

      // No file was given or only contains \n?
      if (strlen(file) == 0 || strlen(file) == 1) {
        error(0);
        return;
      }
    }

    char* args[32];
    char* arg = strtok(command, " \t\n");
    int i = 0;
    while (arg != NULL) {
      if (*arg == ' ' || *arg == '\t')
        arg++;
      else {
        args[i++] = arg;
        arg = strtok(NULL, " \t\n");
      }
    }

    if (i == 0) {
      if (redirect) error(0);
      return;
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
      char** new_path = &args[1];
      write_path(new_path, i - 1);
      return;
    } else if (strcmp(args[0], "printpath") == 0) {
      print_path();
      return;
    }

    if ((forks[pi++] = fork()) == 0) {
      char* full_path;
      for (int i = 0; i < pathlen; i++) {
        full_path = strdup(path[i]);
        strcat(full_path, "/");
        strcat(full_path, args[0]);
        if (access(full_path, X_OK) == 0) {
          if (redirect) {
            close(STDOUT_FILENO);
            open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
          }
          execvp(full_path, args);
        }
      }

      if (redirect) {
        close(STDERR_FILENO);
        open(file, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
      }

      error(1);
    } else {
      // If control reaches parent process, keep creating forks for all child processes
      command = strtok_r(NULL, "&", &saveptr);
      continue;
    }
  }
  // Parent process reached, no more forks are needed, wait for all the children to complete 
  for (int i = 0; i <= pi; i++) {
    waitpid(forks[i], NULL, 0);
  }
}

int main(int argc, const char* argv[]) {
  char* input;
  int bytes_read;
  size_t bufsize = 4096;
  bool batch_mode = false;
  char* init_path[] = {"/bin", "./"};
  write_path(init_path, 2);
  
  if (argc > 2) {
    error(0);
    exit(1);
  }
  
  if (argc > 1) {
    close(STDIN_FILENO);
    open(argv[1], O_RDONLY, S_IRWXU);
    batch_mode = true;
  }

  bool is_empty = true;
  while (true) {
    if (!batch_mode) printf("wish> ");
    input = (char*)malloc(bufsize * sizeof(char));
    bytes_read = getline(&input, &bufsize, stdin);

    if (bytes_read == -1) {
      if (is_empty) {
        error(0);
        exit(1);
      }
      break;
    } else {
      is_empty = false;
    }
    execute(input);
  }
  free(input);
  return 0;
}