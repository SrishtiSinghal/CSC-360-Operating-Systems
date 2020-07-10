//Srishti Singhal
//Programming Assignment 2
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdbool.h>

#define USLEEP_INTERVAL 50000

struct train_t {
  int ID; //file-order
  int loading_time;
  int crossing_time;
  char direction; // east or west
  bool is_ready;
  bool crossed;
} trains[100];

int total_trains;

pthread_cond_t cross[100];
pthread_mutex_t crossMutex[100];
pthread_t threads[100];

pthread_mutex_t finishLoadingMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t finishCrossing = PTHREAD_COND_INITIALIZER;
pthread_mutex_t finishedCrossingMutex = PTHREAD_MUTEX_INITIALIZER;

// 1 tick = 0.1 seconds
int beginningTicks = 0;

int getTicks() {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);

  return now.tv_sec * 10 + now.tv_nsec / 100000000;
}

int timeFromBeginning() {
  return getTicks() - beginningTicks;
}

int getHours(int ticks) {
  return ticks / 36000;
}
int getMins(int ticks) {
  return (ticks % 36000) / 600;
}
int getSecs(int ticks) {
  return (ticks % 600) / 10;
}
int getDsecs(int ticks) {
  return ticks % 10;
}

char east[] = "East";
char west[] = "West";


// definition of a train driver thread
void* train_driver(void* arg) {
  struct train_t* t = arg;

  // direction contains the string for the train's direction
  char* direction = east;
  if (t->direction != 'e' && t->direction != 'E') {
    direction = west;
  }

  pthread_mutex_lock(&crossMutex[t->ID]);

  while (!t->is_ready) {
    if (timeFromBeginning() >= t->loading_time) {
      // take finishLoadingMutex
      pthread_mutex_lock(&finishLoadingMutex);

      int hours = getHours(t->loading_time);
      int mins = getMins(t->loading_time);
      int secs = getSecs(t->loading_time);
      int dsecs = getDsecs(t->loading_time);

      // print loading statement
      printf("%02d:%02d:%02d.%d Train %2d is ready to go %4s\n", hours, mins, secs, dsecs, t->ID, direction);

      t->is_ready = true;

      pthread_mutex_unlock(&finishLoadingMutex);
      // release finishLoadingMutex
    } else {
      usleep(USLEEP_INTERVAL);
    }
  }

  // wait for t->cross condition variable
  pthread_cond_wait(&cross[t->ID], &crossMutex[t->ID]);

  int getting_on_time = timeFromBeginning();
  int hours = getHours(getting_on_time);
  int mins = getMins(getting_on_time);
  int secs = getSecs(getting_on_time);
  int dsecs = getDsecs(getting_on_time);
  printf("%02d:%02d:%02d.%d Train %2d is ON the main track going %4s\n", hours, mins, secs, dsecs, t->ID, direction);
  pthread_mutex_unlock(&crossMutex[t->ID]);

  while (!t->crossed) {
    if (timeFromBeginning() >= t->crossing_time + getting_on_time) {
      t->crossed = true;
      // take finishCrossingMutex
      pthread_mutex_lock(&finishedCrossingMutex);

      int getting_off_time = t->crossing_time + getting_on_time;
      int hours = getHours(getting_off_time);
      int mins = getMins(getting_off_time);
      int secs = getSecs(getting_off_time);
      int dsecs = getDsecs(getting_off_time);

      // print finished corssing statement
      printf("%02d:%02d:%02d.%d Train %2d is OFF the main track after going %4s\n", hours, mins, secs, dsecs, t->ID, direction);

      // signal finishCrossingCV
      pthread_cond_signal(&finishCrossing);

      // release finishCrossingCV
      pthread_mutex_unlock(&finishedCrossingMutex);
    } else {
      usleep(USLEEP_INTERVAL);
    }
  }
  return NULL;
}

bool all_trains_crossed() {
  for (int i=0;i<total_trains;i++) {
    if (!trains[i].crossed) {
      // return false if any train has not crossed yet
      return false;
    }
  }

  // return true if all trains have crossed
  return true;
}

char last_three_trains[] = {'\0', '\0', '\0'};
bool starvation_case(char direction) {
  int i;
  for (i=0; i<3; i++) {
    if (last_three_trains[i] == toupper(direction) || last_three_trains[i] == '\0') {
      // return false if any of the train was in the same direction
      // OR we don't have 3 trains on the track yet
      return false;
    }
  }
  // return true if all three trains were in opposite direction
  return true;
}

bool signal_highest_priority_ready_train() {
  struct train_t* highest_priority_train = NULL;

  for (int i=total_trains-1; i>=0; i--) {
    struct train_t* node = &trains[i];

    if (!node->is_ready || node->crossed) {
      // skip trains that are not ready or have already crossed
      continue;
    }

    if (highest_priority_train == NULL) {
      // if no other train is ready yet, pick this one
      highest_priority_train = node;
      continue;
    }


    // check for starvation
    if (starvation_case(node->direction) && !starvation_case(highest_priority_train->direction)) {
      // pick node if it's solving starvation and highest_priority_train is not
      highest_priority_train = node;
      continue;
    } else if (!starvation_case(node->direction) && starvation_case(highest_priority_train->direction)) {
      // skip node if it's not solving starvation and highest_priority_train is
      continue;
    }

    // pick the highest pri
    if (isupper(node->direction) && islower(highest_priority_train->direction)) {
      // if node has higher priority then pick it
      highest_priority_train = node;
      continue;
    } else if (islower(node->direction) && isupper(highest_priority_train->direction)) {
      // if node is lower priority then skip it
      continue;
    }

    // other rules if same priority

    // same direction case
    if (toupper(node->direction) == toupper(highest_priority_train->direction)) {
      // if both in same direction pick then one with smaller loading time
      if (node->loading_time <= highest_priority_train->loading_time) {
        highest_priority_train = node;
      }
      continue;
    }

    // opposite direction case
    if (last_three_trains[0] == '\0') {
       if (toupper(node->direction) == 'E') {
         // pick eastbound train if no train has crossed yet
         highest_priority_train = node;
      }
    } else if (toupper(node->direction) != last_three_trains[0]) {
      // pick the train travelling opposite to the last train that crossed track
      highest_priority_train = node;
    }
  }

  if (highest_priority_train == NULL) {
    // no train was ready
    return false;
  }

  // update the last_three_trains array
  last_three_trains[2] = last_three_trains[1];
  last_three_trains[1] = last_three_trains[0];
  last_three_trains[0] = toupper(highest_priority_train->direction);

  // take highest_priority_train->crossMutex
  pthread_mutex_lock(&crossMutex[highest_priority_train->ID]);

  // signal highest_priority_train->cross
  pthread_cond_signal(&cross[highest_priority_train->ID]);

  pthread_mutex_lock(&finishedCrossingMutex);
  pthread_mutex_unlock(&crossMutex[highest_priority_train->ID]);
  return true;
}

// definition of operator function
void operator() {
  // sleep for 0.05 seconds
  usleep(USLEEP_INTERVAL);

  while (!all_trains_crossed()) { // start an infinite loop
    bool signalled  = signal_highest_priority_ready_train();
    if (!signalled) {
      // sleep for 0.05 seconds
      usleep(USLEEP_INTERVAL);
      continue;
    }

    // wait for finishCrossing CV
    pthread_cond_wait(&finishCrossing, &finishedCrossingMutex);
    pthread_mutex_unlock(&finishedCrossingMutex);
  }
}

// 3 condition variables or convars -- 1 global to load trains, 1 global for finish crossing and n individual for crossing the track
// 3 mutexes one for loading times, one for the queu (only one train can be added to the queue at a time), one for the track
int main(int argc, char *argv[]) {
  for (int i=0;i<100;i++) {
    pthread_cond_init(&cross[i], NULL);
    pthread_mutex_init(&crossMutex[i], NULL);
  }

  FILE *fp;

  const char* file = argv[1];
  fp = fopen(file, "r");

  if(!fp){
    perror("fopen failed");
    return 1;
  }
  char direction;
  int loading_time;
  int crossing_time;
  total_trains = 0;

  while(fscanf(fp, "%c %d %d\n", &direction, &loading_time, &crossing_time) != EOF){
    struct train_t* node = &trains[total_trains];
    node->ID = total_trains++;
    node->loading_time = loading_time;
    node->crossing_time = crossing_time;
    node->direction = direction;
    node->is_ready = false;
    node->crossed = false;
  }
  fclose(fp);

  beginningTicks = getTicks();

  // spawn all train threads
  for (int i=0;i<total_trains; i++) {
    // spawn a thread
    pthread_create(&threads[i], NULL, train_driver, (void*)&trains[i]);
  }

  // run operator on the main thead
  operator();

}
