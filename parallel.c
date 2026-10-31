#include "lab.h"

void *thread_worker(void *arg) {
  ThreadData *thread_data = (ThreadData *)arg;
  thread_data->wins1 = 0;
  thread_data->wins2 = 0;
  for (long i = 0; i < thread_data->experiments; i++) {
    int score1, score2;
    run_experiment(thread_data->data, &thread_data->seed, &score1, &score2);
    if (score1 > score2) {
      thread_data->wins1++;
    } else if (score2 > score1) {
      thread_data->wins2++;
    }
  }
  return NULL;
}

int run_parallel(ExperimentData *data, int thread_count, long *wins1,
                 long *wins2) {
  pthread_t threads[thread_count];
  ThreadData thread_data[thread_count];
  long base = data->experiments / thread_count;
  long rem = data->experiments % thread_count;
  *wins1 = 0;
  *wins2 = 0;
  for (int i = 0; i < thread_count; i++) {
    thread_data[i].data = data;
    thread_data[i].experiments = base + (i < rem);
    if (get_seed(&thread_data[i].seed) != 0) {
      return -1;
    }
  }
  int created = 0;
  for (int i = 0; i < thread_count; i++) {
    if (pthread_create(&threads[i], NULL, thread_worker, &thread_data[i]) !=
        0) {
      for (int j = 0; j < created; j++) {
        pthread_join(threads[j], NULL);
      }
      return -1;
    }
    created++;
  }
  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
    *wins1 += thread_data[i].wins1;
    *wins2 += thread_data[i].wins2;
  }
  return 0;
}