#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 128

// user input args
int scheduler;
char* trace_filename;
char* output_filename;
int print_events = 0;

// structure representing a process data
struct Process {
  char name[30];
  int arrival_time;
  int dt;
  int deadline;
  int exec_time;
};
// linked list of processes from tracefile
struct ProcessTrace {
  struct Process *current_process;
  struct ProcessTrace *next;
};

void print_linked_list(struct ProcessTrace* trace) {
  struct ProcessTrace* last = trace;
  do {
    printf("\nProcess:\nname: %s", trace->current_process->name);
    printf("\nt0: %d", trace->current_process->arrival_time);
    printf("\ndt: %d", trace->current_process->dt);
    printf("\ndeadline: %d", trace->current_process->deadline);
    printf("\nexec_time: %d", trace->current_process->exec_time);
    last = trace;
    trace = last->next;
  } while (trace->next != NULL);
}

void append_to_linked_list(struct ProcessTrace **head, struct Process* process) {
  struct ProcessTrace* last = *head;
  struct ProcessTrace* trace = malloc(sizeof(struct ProcessTrace*));
  trace->current_process = process;
  trace->next = NULL;
  if (*head == NULL) {
    *head = trace;
    return;
  }

  while (last->next != NULL) {
    last = last->next;
  }
  last->next = trace;
  return;
}

/* Read a process from trace file and create a Process struct
   with the data */
struct Process* current_process(char* line) {
  // extract tokens from tracefile
  char* token;
  char *process_data[4];
  int i = 0;
  while((token = strsep(&line, " ")) != NULL) {
    process_data[i++] = token;
  }
  // allocates space for structs
  struct Process* process = malloc(sizeof(struct Process));
  // assign members of Process with data from tokens extracted from Tracefile
  strcpy(process->name, process_data[0]);
  process->arrival_time = atoi(process_data[1]);
  process->dt = atoi(process_data[2]);
  process->deadline = atoi(process_data[3]);
  process->exec_time = 0;
  return process;
}

/* Build ProcessTrace linked list
  returns head of linked list containing processes sorted by
  arrival time*/
struct ProcessTrace* read_tracefile() {
  FILE* trace_file = fopen(trace_filename, "r");
  if (trace_file == NULL) {
    printf("Could not open this tracefile: %s\n", trace_filename);
    exit(1);
  }
  struct ProcessTrace* head = NULL;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), trace_file)) {
    struct Process* process = current_process(line);
    append_to_linked_list(&head, process);
  }
  fclose(trace_file);
  return head;
}

// reads user input args
void read_args(int argc, char** argv) {
  if (argc < 4) {
    printf("usage: ./ep1 scheduler trace_filename output_filename [-d]\n");
    exit(1);
  }
  scheduler = atoi(argv[1]);
  trace_filename = argv[2];
  output_filename = argv[3];
  if (argc > 4 && strcmp(argv[4], "-d") == 0) {
    print_events = 1;
  }
}

int main(int argc, char** argv) {
  read_args(argc, argv);
  struct ProcessTrace* trace = read_tracefile();
  if (print_events) {
    printf("and I will print the events ;)\n");
  }
  return 0;
}