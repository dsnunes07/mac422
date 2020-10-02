#! /bin/bash

REPEATS=30

SCHEDULER=$1
N_PROCS=$2

echo "run_experiments.sh: escalonador: $SCHEDULER numero de processos: $N_PROCS"

for ((R=1; R <= REPEATS; R++)); do
  ./ep1 ${SCHEDULER} analysis/data/analysis_trace${N_PROCS}_${R} analysis/results/s_${SCHEDULER}_p_${N_PROCS}_out -d 2> analysis/logs/stderr_s_${SCHEDULER}_p_${N_PROCS}_n_${R}
done
