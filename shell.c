#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MSH_RL_BUFSIZE 1024
#define MSH_TOK_SIZE 64
#define MSH_TOK_DELIMETER " \t\r\n\a"

char *msh_read_line(void) {
  int bufsize = MSH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if(!buffer) {
    fprintf(stderr, "msh: allocation error.");
    exit(EXIT_FAILURE);
  }

  while(1) {
    c = getchar();
    
    // if we have eof then just replace it with null character
    if(c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    if(position >= bufsize) {
      bufsize += bufsize;
      buffer = realloc(buffer, bufsize);
      if(!buffer) {
        fprintf(stderr, "msh: allocation error.");
        exit(EXIT_FAILURE);
      }
    }
  }
}

char **msh_split_line(char *line) {
  int bufsize = MSH_TOK_SIZE;
  int position = 0;
  char **tokens = malloc(sizeof(char*) * bufsize);
  char *token;

  if(!tokens) {
    fprintf(stderr, "msh: allocation error.");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, MSH_TOK_DELIMETER);
  while(token != NULL) {
    tokens[position] = token;
    position++;

    if(position >= MSH_TOK_SIZE) {
      bufsize += MSH_TOK_SIZE;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if(!tokens) {
        fprintf(stderr, "msh: allocation error.");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, MSH_TOK_DELIMETER);
  }
  tokens[position] = NULL;
  return tokens;
}

int lsh_launch(char **args) {
  pid_t pid, wpid;
  int status;

  pid = fork();
  if(pid == 0) {
    if(execvp(args[0], args) == -1) {
      perror("msh");
    }
    exit(EXIT_FAILURE);
  } else if(pid < 0) {
    perror("msh");
  } else {
    do {
      wpid = waitpid(pid, &status, WUNTRACED);
    } while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);

char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

int (*builtin_func[]) (char **) = {
  &msh_cd,
  &msh_help,
  &msh_exit
};

int msh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char*);
}

int msh_cd(char **args) {
  if(args[0] == NULL) {
    fprintf(stderr, "msh: expected output to \"cd\"\n");
  } else {
    if(chdir(args[1]) != 0) {
      perror("msh");
    }
  }
  return 1;
}

int msh_help(char **args) {
  int i;
  printf("Sezan's msh.\n");
  printf("Type programs names and arguments. Then hit enter.\n");
  printf("The following are builtin\n");

  for(i = 0; i < msh_num_builtins(); i++) {
    printf(" %s\n", builtin_str[i]);
  }

  printf("Use the man page for to see information for other commands.");
  return 1;
}

int msh_exit(char **args) {
  return 0;
}

int msh_execute(char **args) {
  int i;
  if(args[0] == NULL) {
    return 1;
  }

  for(i = 0; i < msh_num_builtins(); i++) {
    if(strcmp(builtin_str[i], args[0]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return lsh_launch(args);
}

void msh_loop() {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = msh_read_line();
    args = msh_split_line(line);
    status = msh_execute(args);

    free(line);
    free(args);
  } while(status);
  
}

int main() {
  // Load command files, if any
  
  // Running command loop for our shell
  msh_loop();

  // Perform cleanup or shutdown
  return EXIT_SUCCESS;
}


