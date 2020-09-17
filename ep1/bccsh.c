#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <readline/readline.h>
#include <readline/history.h>

// global variables for shell prompt
char* current_user;
char* current_dir;
char* prompt;

// command constants
const char* du = "/usr/bin/du";
const char* traceroute = "/usr/bin/traceroute";
const char* ep1 = "./ep1";


// linked list to store command tokens
struct Node {
  char* data;
  struct Node* next;
};

// Calculates the length of a char array
int length_of(char* str) {
  int i;
  for(i=0; str[i]; i++);
  return i;
}

// Prints the linked list content
void print_linked_list(struct Node* current) {
  while (current != NULL) {
    printf("%s\n", current->data);
    current = current->next;
  }
}

/* calculates the size of a linked list starting at
"current" node in O(n) time. */
int count_linked_list(struct Node* current) {
  int size = 0;
  while (current != NULL) {
    size++;
    current = current->next;
  }
  return size;
}

// creates shell prompt given current user name and current directory
void build_prompt() {
  // calculates size of prompt string to be allocated
  int size = 4 + length_of(current_user) + length_of(current_dir);
  prompt = (char *) malloc(size * sizeof(char));
  prompt[0] = '\0';
  strcat(prompt, "{");
  strcat(prompt, current_user);
  strcat(prompt, "@");
  strcat(prompt, current_dir);
  strcat(prompt, "} ");
}

// split command strings and stores them in a
// linked list
struct Node* split_command(char* line, char* delimiter) {
  char* token;
  struct Node* first_arg = (struct Node*) malloc(sizeof(struct Node));
  first_arg->data = strsep(&line, delimiter);
  first_arg->next = NULL;
  struct Node* previous_arg = first_arg;

  while ((token = strsep(&line, delimiter)) != NULL) {
    struct Node* arg = (struct Node*) malloc(sizeof(struct Node));
    arg->data = token;
    arg->next = NULL;
    previous_arg->next = arg;
    previous_arg = arg;
  }

  return first_arg;
}

const char** copy_to_array(struct Node* args) {
  int argc = count_linked_list(args);
  // debug
  // printf("Number of args: %d\n", argc);
  const char **params = malloc((argc + 1) * sizeof(char *));
  int i = 0;

  while (args != NULL) {
    params[i] = args->data;
    args = args->next;
    // debug
    // printf("params %d == %s\n", i, params[i]);
    i++;
  }

  params[i] = NULL;
  return params;
}

void run_exec_command(const char** args_arr) {
  pid_t pid = fork();
  if (pid == 0) {
    int status = execv(args_arr[0], (char* const*) args_arr);
    if (status) {
      printf("Error while executing command %s\n", args_arr[0]);
    }
    exit(status);
  } else {
    waitpid(pid, NULL, 0);
  }
}

int run_mkdir(const char** args_arr) {
  int status = mkdir(args_arr[1], S_IRWXU);
  if (status) {
    printf("erro: não foi possível criar o diretório '%s'\n", args_arr[1]);
  }
  return status;
}

int run_kill(const char** args_arr) {
  int pid = atoi(args_arr[2]);
  int status = kill(pid, SIGKILL);
  if (status) {
    printf("erro: não foi possível executar kill para o pid %d\n", pid);
  }
  return status;
}

int run_ln(const char** args_arr) {
  const char* from = args_arr[2];
  const char* to = args_arr[3];
  int status = symlink(from, to);
  if (status) {
    printf("erro: não foi possível criar um link simbólico para o arquivo %s\n", from);
  }
  return status;
}
void run_command(const char** args_arr) {
  const char* first_cmd = args_arr[0];
  if (strcmp(first_cmd, du) == 0 || strcmp(first_cmd, traceroute) == 0 || strcmp(first_cmd, ep1) == 0) {
    run_exec_command(args_arr);
  } else {
    if (strcmp(first_cmd, "mkdir") == 0) {
      run_mkdir(args_arr);
    } else if (strcmp(first_cmd, "kill") == 0) {
      run_kill(args_arr);
    } else if (strcmp(first_cmd, "ln") == 0) {
      run_ln(args_arr);
    } else {
      printf("bccsh: command not found.\n");
    }
  }
}

// process text typed by user
void process_input(char* line) {
  char *original_line = strdup(line);
  struct Node* args = split_command(line, " ");
  const char** args_arr = copy_to_array(args);
  run_command(args_arr);
  int argc = count_linked_list(args);
  /* for (int i=0; i < argc; i++) {
    printf("%s\n", args_arr[i]);
  } */

}

void interactive_loop() {
  while(1) {
    char* line = readline(prompt);
    if (line && *line) {
      add_history(line);
      process_input(line);
    }
  }
}

int main(int argc, char** argv) {
  current_user = getlogin();
  current_dir = getcwd(NULL, 0);
  build_prompt();
  using_history();
  interactive_loop();
}
