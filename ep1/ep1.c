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
  pthread_t thread;
  pthread_mutex_t interrupt;
};

// linked list of processes
struct ProcessList {
  struct Process *process;
  struct ProcessList *next;
};

// structure containing simulation data
struct Simulation {
  int seconds_elapsed;
  int context_switches;
};

pthread_mutex_t clock_mutex;

// global simulator clock
struct Simulation* simulation;

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

void list_insert_by_time_left(struct ProcessList **head, struct Process* process) {
  struct ProcessList* list_pointer = *head;
  struct ProcessList* new_proc = malloc(sizeof(struct ProcessList*));
  new_proc->process = process;
  if (list_pointer == NULL || list_pointer->process->dt >= new_proc->process->dt) {
    new_proc->next = *head;
    *head = new_proc;
  } else {
    list_pointer = *head;
    while (list_pointer->next != NULL && list_pointer->process->dt > new_proc->process->dt) {
      list_pointer = list_pointer->next;
    }
    new_proc->next = list_pointer->next;
    list_pointer->next = new_proc;
  }
}

// Removes and returns the first Process of a processlist
struct Process* list_pop(struct ProcessList **head) {
  if (*head == NULL)
    return NULL;
  // get the first element
  struct ProcessList* first_element = *head;
  struct Process* p = first_element->process;
  // makes first element the next of the first
  *head = (*head)->next;
  // free head ref
  free(first_element);
  return p;
}

void *preemptive_worker(void *args) {
  struct Process* process = (struct Process*) args;
  // process executes until the end
  printf("called thread for process %s\n", process->name);
  while((process->dt - process->exec_time) != 0) {
    // but it can be interrupted
    
    pthread_mutex_lock(&(process->interrupt));
    printf("running %s time: %d s\n", process->name, simulation->seconds_elapsed);
    pthread_mutex_lock(&clock_mutex);
    process->exec_time++;
    simulation->seconds_elapsed++;
    pthread_mutex_unlock(&clock_mutex);
    pthread_mutex_unlock(&(process->interrupt));
    sleep(1);
    process->tf = simulation->seconds_elapsed;
    process->tr = process->tf - process->arrival_time;
  }
  printf("%s encerrou seu trabalho\n", process->name);
  return NULL;
}

void *worker(void *args) {
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
      pthread_create(&(process->thread), NULL, worker, process);
      // wait current process to finish
      pthread_join(process->thread, NULL);
      // then move forward on process trace
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
  return p == NULL || p->exec_time == p->dt;
}

void srtn(struct ProcessList* incoming) {
  struct ProcessList* ready = NULL;
  // scheduler clock
  int local_time = -1;
  // currently running process
  struct Process* running = NULL;
  while (incoming != NULL || ready != NULL) {
    pthread_mutex_lock(&clock_mutex);
    local_time = simulation->seconds_elapsed;
    // if a new process has arrived
    if (incoming && local_time == incoming->process->arrival_time) {
      printf("Chegou %s em %d\n", incoming->process->name, local_time);
      if (running == NULL) {
        pthread_create(&(incoming->process->thread), NULL, preemptive_worker, incoming->process);
        running = incoming->process;
        simulation->context_switches++;
      } else {
        // interrupt currently running process
        pthread_mutex_lock(&(running->interrupt));
        printf("Interrompeu processo %s rodando tempo restante: %d para verificar\n", running->name, time_left(running));
        // if new process has shorter dt, keep running process locked
        // and starts the new one. Place older process on ready list
        if (incoming->process->dt < time_left(running)) {
          printf("Deixa %s interrompido por enquanto\n", running->name);
          list_insert_by_time_left(&ready, running);
          running = incoming->process;
          pthread_create(&(incoming->process->thread), NULL, preemptive_worker, running);
          simulation->context_switches++;
        } else {
          printf("Libera %s de novo pois ele pode continuar rodando\n", running->name);
          pthread_mutex_unlock(&(running->interrupt));
          printf("Guarda %s interrompido na fila\n", incoming->process->name);
          pthread_mutex_lock(&(incoming->process->interrupt));
          list_insert_by_time_left(&ready, incoming->process);
        }
      }
      incoming = incoming->next;
      // if no process has arrived but currently running process has finished
    } else if (process_finished(running)) {
      pthread_mutex_unlock(&(running->interrupt));
      if (ready != NULL) {
        struct Process* next_ready = list_pop(&ready);
        printf("Got %s from ready\n", next_ready->name);
        simulation->context_switches++;
        pthread_mutex_unlock(&(next_ready->interrupt));
        running = next_ready;
        // only creates a new thread if process has never run
        if (next_ready->exec_time == 0)
          pthread_create(&(next_ready->thread), NULL, preemptive_worker, next_ready);
      } else {
        // idle state, when a process has finished and no process is waiting,
        // but new processes still on their way
        printf("idle state\n");
        sleep(1);
        simulation->seconds_elapsed++;
      }
    }
    pthread_mutex_unlock(&clock_mutex);
  }
  if (running != NULL) {
    printf("Waiting remaining thread...\n");
    pthread_join(running->thread, NULL);
  }
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
  pthread_mutex_init(&clock_mutex, NULL);
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
  pthread_t process_thread;
  pthread_mutex_t interrupt;
  pthread_mutex_init(&interrupt, NULL);
  process->thread = process_thread;
  process->interrupt = interrupt;
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
