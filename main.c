
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>


char* csh_get_username(void);
int csh_execute(char** args);
int csh_launch(char** args);
char* csh_get_last_argument(char** args);
char** csh_split_line(char* line);
char* csh_read_line(void);
void csh_loop(void);

int csh_cd(char** args);
int csh_help(char** args);
int csh_exit(char** args);

char* builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char**) = {
  &csh_cd,
  &csh_help,
  &csh_exit,
};

int csh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char*);
}


int csh_cd(char** args) {
  if (args[1] == NULL) {
    fprintf(stderr, "csh: expected directory as argument\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("csh");
    }
  }

  return 1;
}

int csh_help(char** args) {
  int i;
  printf("Connor Shell\n-----\n");
  printf("Commands");
  
  for(i = 0; i < csh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }

  return 1;
}


int csh_exit(char** args) {
  return 0;
}


int csh_execute(char** args) {
  if (args[0] == NULL) {
    return 1;
  }

  for (int i = 0; i < csh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return csh_launch(args);
}


int csh_launch(char** args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if (pid == 0) {
    if (execvp(args[0], args) == -1) {
      perror("csh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    perror("csh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

#define CSH_TOK_BUFSIZE 64
#define CSH_TOK_DELIM " \t\r\n\a"
char** csh_split_line(char* line) {
  int bufsize = CSH_TOK_BUFSIZE, position = 0;
  char** tokens = malloc(bufsize * sizeof(char*));
  char* token;

  if (!tokens) {
    fprintf(stderr, "csh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, CSH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += CSH_TOK_BUFSIZE;
      tokens = realloc(tokens, bufsize * sizeof(char));
      if(!tokens) {
        fprintf(stderr, "csh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, CSH_TOK_DELIM);
  }

  tokens[position] = NULL;
  return tokens;
}


#define CSH_RL_BUFSIZE 1024
char* csh_read_line(void) {
  char* line = NULL;
  ssize_t bufsize = 0;
  getline(&line, &bufsize, stdin);
  return line;
}


char* csh_get_username(void) {
  return getlogin();
}


char* csh_get_last_argument(char** args) {

}


void csh_loop(void) {
  char* line;
  char** args;
  char** last_command;
  int status;
  char* username;

  do {

    // get hostname
    char* hostname;
    struct utsname uname_data;
    uname(&uname_data);
    hostname = uname_data.nodename;

    // get username
    username = csh_get_username();

    // print prompt
    printf("[%s@%s] ", username, hostname);

    // get input
    line = csh_read_line();
    args = csh_split_line(line);
    last_command = args;
    status = csh_execute(args);

    free(line);
    free(args);
  } while(status);
}


int main(int argc, char** argv) {
  csh_loop();

  return EXIT_SUCCESS;
}

