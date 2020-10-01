#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_LINE_LENGTH 128

// define flags to identify scheduler events
#define PROCESS_ARRIVED 0
#define PROCESS_RELEASED 1
#define PROCESS_INTERRUPTED 2
#define PROCESS_FINISHED 3
#define CONTEXT_SWITCH 4

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
pthread_mutex_t scheduler_mutex;
pthread_cond_t scheduler_worked;


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

int time_left(struct Process* p) {
  if (p == NULL)
    return 0;

  return p->dt - p->exec_time;
}

void list_insert_by_time_left(struct ProcessList **head, struct Process* process) {
  if (process == NULL)
    return;

  struct ProcessList* current;
  struct ProcessList* new_process = malloc(sizeof(struct ProcessList*));
  new_process->process = process;
  new_process->next = NULL;
  // if arriving process is shorter than the shortest process
  if (*head == NULL || time_left((*head)->process) >= time_left(process)) {
    new_process->next = *head;
    *head = new_process;
  } else {
    current = *head;
    while (current->next != NULL && (time_left(new_process->process) >= time_left(current->next->process))) {
      current = current->next;
    }
    new_process->next = current->next;
    current->next = new_process;
  }
}

// Removes and returns the first Process of a processlist
struct Process* list_pop(struct ProcessList **head) {
  if (*head == NULL) {
    return NULL;
  }
    
  // get the first element
  struct ProcessList* first_element = *head;
  struct Process* p = first_element->process;
  // makes first element the next of the first
  *head = (*head)->next;
  return p;
}

// print events to stderr if user has passed "-d" as command line argument
void print_events_to_stderr(int event, int time, struct Process* p1, struct Process* p2) {
  if (print_events == 0)
    return;

  switch (event) {
  case PROCESS_ARRIVED:
    fprintf(stderr, "[%ds] Chegou no sistema: '%s %d %d %d'\n", time, p1->name, p1->arrival_time, p1->dt, p1->deadline);
    break;
  case PROCESS_RELEASED:
    fprintf(stderr, "[%ds] %s está executando, tempo restante: %ds\n", time, p1->name, time_left(p1));
    break;
  case PROCESS_INTERRUPTED:
    fprintf(stderr, "[%ds] Processo %s (tempo restante: %ds) foi substituído por %s (tempo restante: %ds)\n", time, p1->name, time_left(p1), p2->name, time_left(p2));
    break;
  case PROCESS_FINISHED:
    fprintf(stderr, "[%ds] %s terminou de executar no instante %d s (saída: %s %d %d)\n", time, p1->name, p1->tf, p1->name, p1->tf, p1->tr);
    break;
  default:
    break;
  }
}


void sleep_for(double sec) {
  usleep(sec*1000000);
}

void *time_pass(void *args) {
  while (1) {
    pthread_mutex_lock(&scheduler_mutex);
    pthread_cond_wait(&scheduler_worked, &scheduler_mutex);
    simulation->seconds_elapsed++;
    sleep_for(1);
    pthread_mutex_unlock(&scheduler_mutex);
  }
  return NULL;
}

int get_current_time() {
  int time = 0;
  // pthread_mutex_lock(&clock_mutex);
  time = simulation->seconds_elapsed;
  // pthread_mutex_unlock(&clock_mutex);
  return time;
}

void *preemptive_worker(void *args) {
  struct Process* process = (struct Process*) args;
  int seconds_elapsed = 0;

  // process executes until the end
  while(time_left(process) > 0) {
    // but it can be interrupted
    pthread_mutex_lock(&(process->interrupt));
    pthread_mutex_unlock(&(process->interrupt));
  }
  process->tr = process->tf - process->arrival_time;
  return NULL;
}

int process_finished(struct Process* p) {
  return p->exec_time == p->dt;
}

void start_threads(struct ProcessList* incoming) {
  while (incoming != NULL) {
    struct Process* p = incoming->process;
    pthread_create(&(p->thread), NULL, preemptive_worker, p);
    incoming = incoming->next;
  }
}

void interrupt_process(struct Process* p) {
  if (p == NULL)
    return;

  pthread_mutex_lock(&(p->interrupt));
}

void release_process(struct Process* p) {
  if (p == NULL)
    return;
  pthread_mutex_unlock(&(p->interrupt));
}

void wait_finish(struct Process* p) {
  if (p == NULL)
    return;
  
  pthread_join(p->thread, NULL);
}

void get_processes_arriving_now(int time, struct ProcessList **incoming, struct ProcessList **ready) {
  while (*incoming != NULL && (*incoming)->process->arrival_time == time) {
    list_append(ready, (*incoming)->process);
    print_events_to_stderr(PROCESS_ARRIVED, time, (*incoming)->process, NULL);
    *incoming = (*incoming)->next;
  }
}

// First come first served scheduling algorithm
void fcfs(struct ProcessList* incoming) {
  struct ProcessList* ready = NULL;
  // creates threads for all incoming processes
  start_threads(incoming);
  // starts the clock
  pthread_t clock_thread;
  pthread_create(&clock_thread, NULL, time_pass, NULL);
  // process currently running
  struct Process* running = NULL;
  // keep local_time registered on scheduler
  int local_time = 0;
  // while there are still processes left to run
  while (incoming != NULL || ready != NULL || running != NULL) {
    pthread_mutex_lock(&scheduler_mutex);
    local_time = get_current_time();

    // updates currently running process
    if (running != NULL) {
      print_events_to_stderr(PROCESS_RELEASED, local_time, running, NULL);
      running->exec_time++;
      running->tf = local_time;
    }

    // if a process running has finished, stop it
    if (running != NULL && process_finished(running)) {
      release_process(running);
      wait_finish(running);
      running->exec_time = running->exec_time + 1;
      running->tf = running->tf + 1;
      running->tr = running->tf - running->arrival_time;
      print_events_to_stderr(PROCESS_FINISHED, local_time + 1, running, NULL);
      running = NULL;
    }
    // get all incoming processes for current time
    get_processes_arriving_now(local_time, &incoming, &ready);
    if (running == NULL) {
      // get the next process if theres one
      struct Process* next_ready = list_pop(&ready);
      // and a new process has arrived
      if (next_ready != NULL) {
        // starts arrived process
        release_process(next_ready);
        running = next_ready;
        simulation->context_switches++;
      }
    }
    pthread_cond_signal(&scheduler_worked);
    pthread_mutex_unlock(&scheduler_mutex);
    sleep_for(1);
  }
}

/* Gets all processes arriving in "clock time" and returns the one with the shortest dt,
and inserts any other process in the ready queue */
struct Process* get_shortest_process_at_time(int clock_time, struct ProcessList **incoming, struct ProcessList **ready) {
  int min_dt = INT_MAX;
  struct Process* shortest_process = NULL;
  while (*incoming != NULL && (*incoming)->process->arrival_time == clock_time) {
    struct Process* p = (*incoming)->process;
    print_events_to_stderr(PROCESS_ARRIVED, clock_time, p, NULL);
    // gets only the first shortest process
    if (p->dt < min_dt) {
      // stores the previous shortest process if a faster one has been found
      if (shortest_process != NULL) {
        list_insert_by_time_left(ready, shortest_process);
      }
      min_dt = p->dt;
      shortest_process = p;
    } else {
      list_insert_by_time_left(ready, p);
    }
    *incoming = (*incoming)->next;
  }
  return shortest_process;
}

void srtn(struct ProcessList* incoming) {
  /* list containing processes which have arrived and
  are waiting to run */
  struct ProcessList* ready = NULL;
  
  // scheduler clock variable
  int previous_time = 0;
  int local_time = 0;

  // sleep in nanoseconds
  struct timespec time_nano;
  time_nano.tv_nsec = 1.5*1e9;

  /* start threads of every incoming processes,
  all of them are initially stopped */
  start_threads(incoming);
  
  // currently running process
  struct Process* running = NULL;
  // shortest process arrived at current time
  struct Process* arrived = NULL;

  pthread_t clock_thread;
  pthread_create(&clock_thread, NULL, time_pass, NULL);
  // while exists processes waiting to run
  while (incoming != NULL || ready != NULL || running != NULL) {
    pthread_mutex_lock(&scheduler_mutex);
    local_time = get_current_time();
    if (running != NULL) {
      print_events_to_stderr(PROCESS_RELEASED, local_time, running, NULL);
      running->exec_time++;
      running->tf = local_time;
    }
    // receive a process which may have arrived
    arrived = get_shortest_process_at_time(local_time, &incoming, &ready);
    
    // check if a running process has finished
    if (running != NULL && process_finished(running)) {
      // let thread exit if necessary
      release_process(running);
      wait_finish(running);
      running->exec_time = running->exec_time + 1;
      running->tf = running->tf + 1;
      running->tr = running->tf - running->arrival_time;
      // print to stderr if user requested
      print_events_to_stderr(PROCESS_FINISHED, local_time + 1, running, NULL);
      // process is no longer running
      running = NULL;
    }

    if (running != NULL) {
      // interrupt current process
      interrupt_process(running);

      // if no process has arrived
      if (arrived == NULL) {
        // process can continue running
        release_process(running);
        // if new process is shortest than currently running process
      } else if (arrived->dt < time_left(running) + 1) {
        running->exec_time = running->exec_time - 1;
        list_insert_by_time_left(&ready, running);
        print_events_to_stderr(PROCESS_INTERRUPTED, simulation->seconds_elapsed, running, arrived);
        release_process(arrived);
        running = arrived;
        simulation->context_switches++;
      } else {
        // current process still faster than the new one
        list_insert_by_time_left(&ready, arrived);
        release_process(running);
      }
    } else {
      if (arrived != NULL) {
        struct Process* next_ready = list_pop(&ready);
        if (next_ready != NULL && next_ready->dt < arrived->dt) {
          list_insert_by_time_left(&ready, arrived);
          release_process(next_ready);
          running = next_ready;
          simulation->context_switches++;
        } else {
          list_insert_by_time_left(&ready, next_ready);
          release_process(arrived);
          running = arrived;
          simulation->context_switches++;
        }
      } else {
        struct Process* next_ready = list_pop(&ready);
        if (next_ready != NULL) {
          release_process(next_ready);
          running = next_ready;
          simulation->context_switches++;
        }
      }
    }
    pthread_cond_signal(&scheduler_worked);
    pthread_mutex_unlock(&scheduler_mutex);
    sleep_for(0.5);
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
  pthread_mutex_init(&scheduler_mutex, NULL);
  pthread_cond_init(&scheduler_worked, NULL);
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
  // all processes starts interrupted
  pthread_mutex_lock(&interrupt);
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
    printf("Fim da execução. Dados salvos no arquivo %s\n", output_filename);
  }
  return 0;
}
