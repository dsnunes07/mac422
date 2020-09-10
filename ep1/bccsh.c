#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

char* current_user;
char* current_dir;
char* prompt;

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
  int size = length_of(current_user) + length_of(current_dir) + 4;
  prompt = (char *) malloc(size * sizeof(char));
  strcat(prompt, "{");
  strcat(prompt, current_user);
  strcat(prompt, "@");
  strcat(prompt, current_dir);
  strcat(prompt, "} ");
}

// split command strings and stores them in a
// linked list
struct Node* split_command(char* line) {
  char* token;
  struct Node* first_arg = (struct Node*) malloc(sizeof(struct Node));
  first_arg->data = strsep(&line, " ");
  struct Node* previous_arg = first_arg;

  while ((token = strsep(&line, " ")) != NULL) {
    struct Node* arg = (struct Node*) malloc(sizeof(struct Node));
    arg->data = token;
    arg->next = NULL;
    previous_arg->next = arg;
    previous_arg = arg;
  }

  return first_arg;
}

// process text typed by user
void process_command(char* line) {
  char *original_line = strdup(line);
  struct Node* args = split_command(line);
  int size = count_linked_list(args);
  printf("Number of args: %d\n", size);
  print_linked_list(args);
}

void interactive_loop() {
  while(1) {
    char* line = readline(prompt);
    if (line && *line) {
      add_history(line);
      process_command(line);
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
