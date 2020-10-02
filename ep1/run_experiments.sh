#! /bin/bash

REPEATS=30
N_PROCS=(10 100 1000)

# for each scheduler...
# 2 SCHEDULERS ALREADY IMPLEMENTED
for ((SCHEDULER=1; SCHEDULER <= 2; SCHEDULER++ )); do
  echo "Escalonador: $SCHEDULER"
  # for each amount of processes
  for N in ${N_PROCS[@]}; do
    echo "Quantidade de processos: $N"
    for ((R=1; R <= REPEATS; R++)); do
      # ./ep1 ${SCHEDULER} analysis/data/analysis_trace${N} analysis/results/s_${SCHEDULER}_p_${N}_n_${R}_out -d 2> analysis/logs/stderr_s_${SCHEDULER}_p_${N}_n_${R}
      ./ep1 ${SCHEDULER} analysis/data/trace-test analysis/results/s_${SCHEDULER}_p_${N}_n_${R}_out -d 2> analysis/logs/stderr_s_${SCHEDULER}_p_${N}_n_${R}
    done
  done
done
