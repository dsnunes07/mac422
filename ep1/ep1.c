#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

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
  int tf;
  int tr;
  int exec_time;
};
// linked list of processes from tracefile
struct ProcessList {
  struct Process *process;
  struct ProcessList *next;
};

struct Simulation {
  int seconds_elapsed;
  int context_switches;
};

// global simulator clock
struct Simulation* simulation;

// Mutexes
pthread_mutex_t interrupt_process_mutex;

int interrupt_running_process = 0;

// Process list traversal to print current processes
void print_linked_list(struct ProcessList* trace) {
  while (trace != NULL) {
    printf("\nProcess:\nname: %s", trace->process->name);
    printf("\nt0: %d", trace->process->arrival_time);
    printf("\ndt: %d", trace->process->dt);
    printf("\ndeadline: %d", trace->process->deadline);
    printf("\nexec_time: %d", trace->process->exec_time);
    printf("\n%d\n", trace->next == NULL);
    trace = trace->next;
  }
}

// Append a process to the end of a linked list pointed by head
void list_append(struct ProcessList **head, struct Process* process) {
  struct ProcessList* last = *head;
  struct ProcessList* trace = malloc(sizeof(struct ProcessList*));
  trace->process = process;
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

void list_push(struct ProcessList **head, struct Process* process) {
  struct ProcessList* new_head = (struct ProcessList*) malloc(sizeof(struct ProcessList*));
  new_head->process = process;
  new_head->next = *head;
  *head = new_head;
}

void *process_preemptive_work(void *args) {
  struct Process* process = (struct Process*) args;
  int exec_time = 0;
  while (process->dt - process->exec_time != 0) {
    sleep(1);
    simulation->seconds_elapsed++;
    process->exec_time++;
    if (interrupt_running_process) {
      pthread_mutex_lock(&interrupt_process_mutex);
      interrupt_running_process = 0;
      pthread_mutex_unlock(&interrupt_process_mutex);
      break;
    }
  }
}

void *process_work(void *args) {
  struct Process* process = (struct Process*) args;
  int dt = process->dt;
  int exec_time = 0;
  while (dt - exec_time != 0) {
    sleep(1);
    simulation->seconds_elapsed++;
    exec_time++;
  }
  // process finish executing
  process->exec_time = exec_time;
  process->tf = simulation->seconds_elapsed;
  process->tr = process->tf - process->arrival_time;
}

// First come first served scheduling algorithm
void fcfs(struct ProcessList* incoming) {
  while (incoming != NULL) {
    struct Process* process = incoming->process;
    if (process->arrival_time <= simulation->seconds_elapsed) {
      // execute current process until it finishes
      simulation->context_switches++;
      pthread_t process_thread;
      pthread_create(&process_thread, NULL, process_work, process);
      pthread_join(process_thread, NULL);
      // move forward on process trace
      incoming = incoming->next;
    } else {
      // keep waiting for a new process
      simulation->seconds_elapsed++;
      sleep(1);
    }
  }
}

int time_left(struct Process* p) {
  return p->dt - p->exec_time;
}

int process_finished(struct Process* p) {
  return p->exec_time == p->dt;
}

void srtn(struct ProcessList* incoming) {
  struct ProcessList* ready = malloc(sizeof(struct ProcessList*));
  struct Process* running = NULL;
  pthread_mutex_init(&interrupt_process_mutex, NULL);
  pthread_t running_thread;
  // schedule while there are process left to run
  while (incoming != NULL) {
    // printf(">> %s %d s\n", running->name, simulation->seconds_elapsed);
    // next process has arrived
    if (incoming->process->arrival_time == simulation->seconds_elapsed) {
      struct Process* next_proc = incoming->process;
      // if a process is already running
      if (running != NULL) {
        // check if current process will replace the active process
        if (next_proc->dt < (running->dt - running->exec_time)) {
          pthread_mutex_lock(&interrupt_process_mutex);
          interrupt_running_process = 1;
          pthread_mutex_unlock(&interrupt_process_mutex);
          pthread_create(&running_thread, NULL, process_preemptive_work, next_proc);
          running = next_proc;
        } 
      } else {
        pthread_create(&running_thread, NULL, process_preemptive_work, next_proc);
        running = next_proc;
      }
      // once next process is handled, move forward on incoming list
      incoming = incoming->next;
    } else if (running == NULL) {
      sleep(1);
      simulation->seconds_elapsed++;
    }
  }
  pthread_join(running_thread, NULL);
}

void print_output_file(struct ProcessList* trace) {
  FILE *fp = fopen(output_filename, "w");
  while (trace != NULL) {
    struct Process *p = trace->process;
    fprintf(fp, "%s %d %d\n", p->name, p->tf, p->tr);
    trace = trace->next;
  }
  fprintf(fp, "%d", simulation->context_switches);
  fclose(fp);
}

void simulate(struct ProcessList* trace) {
  // initialize a new clock for simulation
  simulation = malloc(sizeof(struct Simulation*));
  simulation->seconds_elapsed = 0;
  simulation->context_switches = 0;
  if (scheduler == 1) {
    fcfs(trace);
  } else if (scheduler == 2) {
    srtn(trace);
  }

  print_output_file(trace);
}

/* Read a process from trace file, create and returns a Process struct
  containing its data */
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

/* Build ProcessList linked list
  returns head of linked list containing processes sorted by
  arrival time */
struct ProcessList* read_tracefile() {
  FILE* trace_file = fopen(trace_filename, "r");
  if (trace_file == NULL) {
    printf("Could not open this tracefile: %s\n", trace_filename);
    exit(1);
  }
  struct ProcessList* head = NULL;
  char line[MAX_LINE_LENGTH];
  while (fgets(line, sizeof(line), trace_file)) {
    struct Process* process = current_process(line);
    list_append(&head, process);
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
  struct ProcessList* processes = read_tracefile();
  simulate(processes);
  if (print_events) {
    printf("and I will print the events ;)\n");
  }
  return 0;
}
