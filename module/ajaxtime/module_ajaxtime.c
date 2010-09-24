#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "chesstalk_module.h"

struct time_pack {
  int min, sec;
};

char *json_file_default = "ajaxtime.json";

#include <pthread.h>

#define THREAD_STARTED 0x1
#define THREAD_ACTIVE 0x2
#define IDLE_WRITE 0x4

struct work_items {

  char *json_file;
  int fd;

  struct time_pack w_time, b_time;

  int white_increment, black_increment;

  struct timeval game_start;

  struct timeval tv_prior, tv_recent;

  int white_move;
  int move_number;
  char move_string[10];

  int state;

  pthread_t idle_thread_writer;

  pthread_mutex_t update_lock, submission_lock, threadstate_lock;

  char json[80];

};

struct work_items w;

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <unistd.h>

int write_json_file(char *json_string) {

  off_t seek;

  //  printf("%s: Writing json_file.\n", __FUNCTION__);

  if (w.fd == -1) {

    w.fd = open(w.json_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
    if (w.fd == -1) {
      perror("open");
      printf("%s: Trouble opening json_file=%s for writing.\n", __FUNCTION__, w.json_file);
      return -1;
    }

  }  

  seek = lseek(w.fd, 0, SEEK_SET);

  if (seek != -1) {

    ssize_t written_bytes;

    off_t file_length = strlen(json_string);

    written_bytes = write(w.fd, json_string, file_length);

    if (written_bytes != file_length) {

      if (written_bytes == -1) {
	perror("write");
	return -1;
      }

      else {
	printf("%s: Partial write. Expected %ld but got %ld.\n", __FUNCTION__, file_length, written_bytes);
	return -1;
      }

    }

    if (ftruncate(w.fd, file_length) == -1) {
      
      perror("ftruncate");
      return -1;

    }

    if (fdatasync(w.fd) != 0) {
      
      perror("fdatasync");
      return -1;

    }
    

  }

  else {

    perror("seek");

    printf("%s: Trouble with seek.\n", __FUNCTION__);

    return -1;

  }

  return 0;

}

int apply_trim(struct time_pack *trim_amount, struct time_pack *cleave_pointer) {

  assert(cleave_pointer!=NULL);

  assert(trim_amount!=NULL);

  cleave_pointer->min -= trim_amount->min;
  cleave_pointer->sec -= trim_amount->sec;

  if (cleave_pointer->sec < 0) {
    cleave_pointer->sec += 60;
    cleave_pointer->min -= 1;
  }

  if (cleave_pointer->min < 0 || cleave_pointer->sec < 0) {
    cleave_pointer->min = 0;
    cleave_pointer->sec = 0;
  }

}

int second_compute_trim(time_t start, time_t current, struct time_pack *trim_amount) {

  assert(trim_amount!=NULL);

  trim_amount->min = current - start > 0 ? (current - start) / 60 : 0;

  trim_amount->sec = (current - start) - trim_amount->min * 60;

}

int reformulate_json(char *string, struct time_pack *w_time, struct time_pack *b_time, int move_number, char *move_string, int white_move) {

  assert(string!=NULL && move_string!=NULL);

  assert(w_time!=NULL && b_time!=NULL);

  sprintf(string, "{ \"w_time\": \"%d:%02d\", \"b_time\": \"%d:%02d\", \"last_move\": \"%d. %s%s\" }\n", 
	  w_time->min, w_time->sec, b_time->min, b_time->sec, move_number, white_move?"":"...", move_string);

  return 0;

}

int update_json() {

  struct time_pack *cleave_pointer;

  struct time_pack trim_amount;

  struct timeval now;

  char string[80];

  int retval;

  pthread_mutex_lock(&w.submission_lock);

  pthread_mutex_lock(&w.update_lock);

  second_compute_trim(w.tv_prior.tv_sec, w.tv_recent.tv_sec, &trim_amount);

  cleave_pointer = w.white_move ? &w.b_time : &w.w_time;

  trim_amount.min = 0;
  trim_amount.sec = 1;

  apply_trim(&trim_amount, cleave_pointer);

  reformulate_json(string, &w.w_time, &w.b_time, w.move_number, w.move_string, w.white_move);

  pthread_mutex_unlock(&w.update_lock);

  pthread_mutex_unlock(&w.submission_lock);

  assert(sizeof(w.json) == sizeof(string));

  memcpy(w.json, string, sizeof(w.json));

  retval = write_json_file(w.json);

  if (retval==-1) {
    printf("%s: Trouble with write for json_file.\n", __FUNCTION__);
    return -1;
  }

  return 0;

}


int move_submission(int move_number, char *move_string, int white_move) {

  struct timeval now;

  int retval;

  assert(move_string!=NULL);

  pthread_mutex_lock(&w.threadstate_lock);
  if (w.state & IDLE_WRITE) {
    if (w.state & THREAD_STARTED) {
      w.state = THREAD_STARTED | THREAD_ACTIVE;
    }
  }
  pthread_mutex_unlock(&w.threadstate_lock);

  pthread_mutex_lock(&w.update_lock);

  w.move_number = move_number;
  strcpy(w.move_string, move_string);
  w.white_move = white_move;

  gettimeofday(&now, NULL);
  memcpy(&w.tv_prior, &w.tv_recent, sizeof(struct timeval));
  memcpy(&w.tv_recent, &now, sizeof(struct timeval));

  pthread_mutex_unlock(&w.update_lock);

  pthread_mutex_lock(&w.threadstate_lock);
  w.state |= IDLE_WRITE;
  pthread_mutex_unlock(&w.threadstate_lock);

  return 0;

}

void *idle_writer(void *argument) {

  void *result = NULL;

  int debug = 0;

  printf("%s: Starting json writer thread.\n", __FUNCTION__);

  for ( ;; ) {

    pthread_mutex_lock(&w.threadstate_lock);
    if (!(w.state&THREAD_ACTIVE)) {
      if (debug>=3) {
	printf("%s: w.state changed to %d and need at least %d, so leaving.\n", __FUNCTION__, w.state, THREAD_ACTIVE);
      }
      pthread_mutex_unlock(&w.threadstate_lock);
      return result;
    }      
    pthread_mutex_unlock(&w.threadstate_lock);    

    usleep(1000 * 1000);

    if (debug>=3) {
      printf("%s: Sleeping.\n", __FUNCTION__);
    }

    pthread_mutex_lock(&w.threadstate_lock);
    if (w.state & IDLE_WRITE) {

      if (debug>=3) {
	printf("%s: Reforumalting for write.\n", __FUNCTION__);
      }

      update_json();

      //      write_json_file(w.json);

    }
    pthread_mutex_unlock(&w.threadstate_lock);

  }

}

int module_shutdown() {
  
  int retval;

  pthread_mutex_lock(&w.threadstate_lock);
  w.state = 0;
  pthread_mutex_unlock(&w.threadstate_lock);

  pthread_join(w.idle_thread_writer, NULL);

  pthread_mutex_destroy(&w.update_lock);
  pthread_mutex_destroy(&w.submission_lock);
  pthread_mutex_destroy(&w.threadstate_lock);

  printf("%s: w.fd=%d\n", __FUNCTION__, w.fd);

  if (w.fd != -1) {

    retval = close(w.fd);
    if (retval != 0) {
      perror("close");
      printf("%s: Trouble with close operation on w.fd=%d errno=%d\n", __FUNCTION__, w.fd, errno);
      return -1;
    }

  }

  return 0;

}

// example: WHITE_TIME=8 BLACK_TIME=8 WHITE_INCREMENT=10 BLACK_INCREMENT=10 JSON_FILE=/var/www/nginx-default/ajaxtime/ajaxtime.json CHESSTALK_MODULE=libchesstalk-module.so ./chesstalk

int module_entry(struct chesstalk_module *m) {

  char *starting_white_time = getenv("WHITE_TIME");

  char *starting_black_time = getenv("BLACK_TIME");

  char *white_increment_seconds = getenv("WHITE_INCREMENT");

  char *black_increment_seconds = getenv("BLACK_INCREMENT");

  int default_white_time_min = 5;
  int default_white_time_sec = 30;
  int default_black_time_min = 5;
  int default_black_time_sec = 30;

  int default_white_increment_seconds = 0;
  int default_black_increment_seconds = 0;

  char *sec_work;

  int rc;

  if (m==NULL) {
    printf("%s: Need an allocated chesstalk_module structure.\n", __FUNCTION__);
    return -1;
  }

  m->move_submission = move_submission;
  m->module_shutdown = module_shutdown;

  w.json_file = getenv("JSON_FILE");

  if (w.json_file==NULL) w.json_file = json_file_default;


  w.w_time.min = starting_white_time!=NULL ? strtol(starting_white_time, NULL, 10) : default_white_time_min;
  sec_work = strchr(starting_white_time, ':');
  w.w_time.sec = sec_work!=NULL ? strtol(sec_work+1, NULL, 10) : default_white_time_sec;

  w.b_time.min = starting_black_time!=NULL ? strtol(starting_black_time, NULL, 10) : default_black_time_min;
  sec_work = strchr(starting_black_time, ':');
  w.b_time.sec = sec_work!=NULL ? strtol(sec_work+1, NULL, 10) : default_black_time_sec;

  w.white_increment = white_increment_seconds != NULL ? strtol(white_increment_seconds, NULL, 10) : default_white_increment_seconds;
  w.black_increment = black_increment_seconds != NULL ? strtol(black_increment_seconds, NULL, 10) : default_black_increment_seconds;

  w.fd = -1;

  w.state = 0;

  gettimeofday(&w.game_start, NULL);

  memcpy(&w.tv_prior, &w.game_start, sizeof(struct timeval));
  memcpy(&w.tv_recent, &w.game_start, sizeof(struct timeval));

  printf("%s: Using json_file=%s\n", __FUNCTION__, w.json_file);

  pthread_mutex_init(&w.update_lock, NULL);
  pthread_mutex_init(&w.submission_lock, NULL);
  pthread_mutex_init(&w.threadstate_lock, NULL);

  pthread_mutex_lock(&w.threadstate_lock);
  w.state |= THREAD_ACTIVE;
  pthread_mutex_unlock(&w.threadstate_lock);

  rc = pthread_create(&w.idle_thread_writer, NULL, &idle_writer, NULL);

  if (rc != 0) {
    printf("%s: Trouble creating idle_writer_thread.\n", __FUNCTION__);
  }

  pthread_mutex_lock(&w.threadstate_lock);
  w.state |= THREAD_STARTED;
  pthread_mutex_unlock(&w.threadstate_lock);

  return 0;

}

