#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Function declarations
*/
void psh_loop(void);
char *psh_read_line(void);
char **psh_split_line(char *line);
int psh_launch(char **args);
int psh_execute(char **args);

void psh_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    // Display a prompt to the user
    printf("> ");
    // Read the command from standard input
    line = psh_read_line();
    // Separate the command string into a program and arguments
    args = psh_split_line(line);
    // Run the parsed command
    status = psh_execute(args);

    free(line);
    free(args);
  } while(status);
}

#define PSH_RL_BUFSIZE 1024
char *psh_read_line(void) {
  int bufsize = PSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  // check that memory has been allocated
  if (!buffer) {
    fprintf(stderr, "psh: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    // Read a character
    c = getchar();

    // If we hit EOF, replace it with a null character and return
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }
    else {
      buffer[position] = c;
    }
    position++;

    // If we have exceeded the buffer size, reallocate memory
    if (position >= bufsize) {
      bufsize += PSH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);                                     // Check on this line
      if (!buffer) {
        fprintf(stderr, "psh: memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define PSH_TOK_BUFSIZE 64;
#define PSH_TOK_DELIM " \t\r\n\a"
char **psh_split_line(char *line) {
  int bufsize = PSH_TOK_BUFSIZE;
  int position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;

  if (!tokens) {
    fprintf(stderr, "psh: memory allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, PSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += PSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
        fprintf(stderr, "psh: memory allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, PSH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}

int psh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    // Child process
    if (execvp(args[0], args) == -1) {
      perror("psh");
    }
    exit(EXIT_FAILURE);
  }
  else if (pid < 0) {
  // Error forking
  perror("psh");
  }
  else {
    // Parent process
    do {
      wpid = waitpid(pid,&status,WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

/* Function declarations for builtin shell commands
*/
int psh_cd(char **args);
int psh_help(char **args);
int psh_exit(char **args);

/* List of builtin commands, followed by their corresponding functions
*/
char *builtin_str[] = {"cd","help","exit"};

int (*builtin_func[]) (char **) = {&psh_cd, &psh_help, &psh_exit};

int psh_num_builtins() {
  return sizeof(builtin_str)/sizeof(char *);
}

/* implementations of builtin functions
*/
int psh_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "psh: expected argument to \"cd\"\n");
  }
  else {
    if (chdir(args[1]) != 0) {
      perror("psh");
    }
  }
  return 1;
}

int psh_help(char **args) {
  int i;
  printf("Prosper's PSH\n");
  printf("Type program names and arguments and hit enter.\n");
  printf("The following are built in:\n");

  for (int i = 0; i < psh_num_builtins(); i++) {
    printf(" %s\n", builtin_str[i]);
  }
  printf("Use the main command for information on other programs.\n");
  return 1;
}

int psh_exit(char **args) {
  return 0;
}

int psh_execute(char **args) {
  int i;
  if (args[0] == NULL) {
    // An empty command was entered
    return 1;
  }

  for (i=0; i < psh_num_builtins(); i++) {
    if (strcmp(args[0],builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }
  return psh_launch(args);
}

int main(int argc, char **argv) {
  // Load config files, if any

  // Run command loop
  psh_loop();

  // Perform any shutdown/cleanup

  return EXIT_SUCCESS;
}
