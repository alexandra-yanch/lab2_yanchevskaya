#ifndef LAB_H
#define LAB_H

#include <fcntl.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int k;
  int curr_turn;
  int score1;
  int score2;
  long experiments;
} ExperimentData;

typedef struct {
  ExperimentData *data;
  long experiments;
  long wins1;
  long wins2;
  unsigned int seed;
} ThreadData;

void print_error(void);
int read_num(long *num);
int get_seed(unsigned int *seed);
int roll_dice(unsigned int *seed);
void play_round(unsigned int *seed, int *score1, int *score2);

void run_experiment(ExperimentData *data, unsigned int *seed, int *score1,
                    int *score2);

int run_seq(ExperimentData *data, long *wins1, long *wins2);
int run_parallel(ExperimentData *data, int thread_count, long *wins1,
                 long *wins2);

double get_time(struct timespec start, struct timespec end);

void *thread_worker(void *arg);

#endif