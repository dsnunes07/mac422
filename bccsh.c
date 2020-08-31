#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

char* current_user;
char* current_dir;
char* prompt;
char cmd[8];

char* receive_cmd() {
  char *line = readline(prompt);
  return line;
}

int length_of(char* str) {
  int i;
  for(i=0; str[i]; i++);
  return i;
}

void build_prompt() {
  int size = length_of(current_user) + length_of(current_dir) + 4;
  prompt = (char *) malloc(size * sizeof(char));
  strcat(prompt, "{");
  strcat(prompt, current_user);
  strcat(prompt, "@");
  strcat(prompt, current_dir);
  strcat(prompt, "} ");
}

void interactive_loop() {
  while(1) {
    char* line = receive_cmd();
    if (line && *line) {
      add_history(line);
    }
    printf("%s\n", line);
  }
}

int main(int argc, char** argv) {
  current_user = getlogin();
  current_dir = getcwd(NULL, 0);
  build_prompt();
  using_history();
  interactive_loop();
}
