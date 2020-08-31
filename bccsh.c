#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <readline/history.h>

char* current_user;
char* current_dir;
char cmd[8];

void print_shell_prompt() {
  printf("{%s@%s} ", current_user, current_dir);
}

char* receive_cmd() {
  fgets(cmd, sizeof(cmd), stdin);
  return cmd;
}

void interactive_loop() {
  while(1) {
    print_shell_prompt();
    char* cmd = receive_cmd();
    add_history(cmd);
    printf("history length: %d\n", history_length);
    printf("%s", current_history()->line);
  }
}



int main(int argc, char** argv) {
  current_user = getlogin();
  current_dir = getcwd(NULL, 0);
  using_history();
  interactive_loop();
}
