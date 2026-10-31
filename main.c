#include "lab.h"

void print_error(void) { write(2, "error\n", 6); }

int num_to_str(long num, char *str) {
  int i = 0;
  if (num == 0) {
    str[i++] = '0';
  }
  while (num > 0) {
    str[i++] = num % 10 + '0';
    num /= 10;
  }
  str[i] = '\0';
  for (int j = 0; j < i / 2; j++) {
    char temp = str[j];
    str[j] = str[i - j - 1];
    str[i - j - 1] = temp;
  }
  return i;
}

int read_num(long *num) {
  char buff[32];
  long value = 0;
  int i = 0;
  int bytes = read(0, buff, sizeof(buff));
  if (bytes <= 0) {
    return -1;
  }
  while (i < bytes && buff[i] >= '0' && buff[i] <= '9') {
    value = value * 10 + buff[i] - '0';
    i++;
  }
  if (i == 0 || (i < bytes && buff[i] != '\n')) {
    return -1;
  }
  *num = value;
  return 0;
}

int get_seed(unsigned int *seed) {
  int fd = open("/dev/urandom", O_RDONLY);
  if (fd < 0) {
    return -1;
  }
  int bytes = read(fd, seed, sizeof(*seed));
  close(fd);
  if (bytes != sizeof(*seed)) {
    return -1;
  }
  return 0;
}

int roll_dice(unsigned int *seed) {
  *seed = *seed * 1103515245 + 12345;
  return (*seed % 6) + 1;
}

void play_round(unsigned int *seed, int *score1, int *score2) {
  int p1 = roll_dice(seed) + roll_dice(seed);
  int p2 = roll_dice(seed) + roll_dice(seed);
  if (p1 > p2) {
    (*score1)++;
  } else if (p2 > p1) {
    (*score2)++;
  }
}

void run_experiment(ExperimentData *data, unsigned int *seed, int *score1,
                    int *score2) {
  *score1 = data->score1;
  *score2 = data->score2;
  int remain_turns = data->k - data->curr_turn;
  for (int i = 0; i < remain_turns; i++) {
    play_round(seed, score1, score2);
  }
}

int run_seq(ExperimentData *data, long *wins1, long *wins2) {
  unsigned int seed;
  if (get_seed(&seed) != 0) {
    return -1;
  }
  *wins1 = 0;
  *wins2 = 0;
  for (long i = 0; i < data->experiments; i++) {
    int score1, score2;
    run_experiment(data, &seed, &score1, &score2);
    if (score1 > score2) {
      (*wins1)++;
    } else if (score2 > score1) {
      (*wins2)++;
    }
  }
  return 0;
}

double get_time(struct timespec start, struct timespec end) {
  return (end.tv_sec - start.tv_sec) * 1000.0 +
         (end.tv_nsec - start.tv_nsec) / 1000000.0;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    print_error();
    return 1;
  }
  long thread_value = 0;
  int i = 0;
  while (argv[1][i] >= '0' && argv[1][i] <= '9') {
    thread_value = thread_value * 10 + argv[1][i] - '0';
    i++;
  }
  if (i == 0 || argv[1][i] != '\0' || thread_value <= 0) {
    print_error();
    return 1;
  }
  int thread_count = (int)thread_value;
  ExperimentData data;
  long value;
  if (read_num(&value) != 0 || value <= 0) {
    print_error();
    return 1;
  }
  data.k = (int)value;
  if (read_num(&value) != 0 || value < 0 || value > data.k) {
    print_error();
    return 1;
  }
  data.curr_turn = (int)value;
  if (read_num(&value) != 0) {
    print_error();
    return 1;
  }
  data.score1 = (int)value;
  if (read_num(&value) != 0) {
    print_error();
    return 1;
  }
  data.score2 = (int)value;
  if (read_num(&value) != 0 || value <= 0) {
    print_error();
    return 1;
  }
  data.experiments = value;
  long wins1, wins2;
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (run_seq(&data, &wins1, &wins2) != 0) {
    print_error();
    return 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  double seq_time = get_time(start, end);
  long parallel_wins1, parallel_wins2;
  clock_gettime(CLOCK_MONOTONIC, &start);
  if (run_parallel(&data, thread_count, &parallel_wins1, &parallel_wins2) !=
      0) {
    print_error();
    return 1;
  }
  clock_gettime(CLOCK_MONOTONIC, &end);
  double parallel_time = get_time(start, end);
  char buff[64];
  int len;

  len = num_to_str(wins1, buff);
  write(1, "Seq1: ", 6);
  write(1, buff, len);
  write(1, "\n", 1);

  len = num_to_str(wins2, buff);
  write(1, "Seq2: ", 6);
  write(1, buff, len);
  write(1, "\n", 1);

  len = num_to_str(parallel_wins1, buff);
  write(1, "Par-lel1: ", 10);
  write(1, buff, len);
  write(1, "\n", 1);

  len = num_to_str(parallel_wins2, buff);
  write(1, "Par-lel2: ", 10);
  write(1, buff, len);
  write(1, "\n", 1);

  len = num_to_str((long)seq_time, buff);
  write(1, "Seq time: ", 10);
  write(1, buff, len);
  write(1, "\n", 1);

  len = num_to_str((long)parallel_time, buff);
  write(1, "Par-lel time: ", 14);
  write(1, buff, len);
  write(1, "\n", 1);

  return 0;
}