//Group Members: Hyrum Sorensen, Ashley Wagner, Jarod Whiting, Spencer Ream
#define _DEFAULT_SOURCE
#include <assert.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>

// total number of sharks and divers
const int SHARK_COUNT = 6;
const int DIVER_COUNT = 2;

// capacity of the reef (this many sharks OR divers)
const int MAX_IN_REEF = 2;

// max time a shark waits before getting hungry (in microseconds)
const int SHARK_WAITING_TIME = 2000000;

// max time a shark spends feeding in the reef
const int SHARK_FISHING_TIME = 2000000;

// max time a diver waits before wanting to fish
const int DIVER_WAITING_TIME = 200000;

// max time a diver spends fishing in the reef
const int DIVER_FISHING_TIME = 2000000;

// total time the simulation should run (in seconds)
const int TOTAL_SECONDS = 60;

// whether or not each shark/diver is currently in the reef
bool *divers_fishing;
bool *sharks_feeding;


int waiting_sharks = 0;
int waiting_divers = 0;

// declare synchronization variables here
sem_t reef_semaphore;
// end synchronization variables

void report(void) {
    // shark report
    int total_sharks = 0;
    char shark_report[100];
    shark_report[0] = 0;

    for (int i = 0; i < SHARK_COUNT; i++) {
        if (sharks_feeding[i]) {
            strcat(shark_report, "+");
            total_sharks++;
        } else {
            strcat(shark_report, " ");
        }
    }

    // diver report
    int total_divers = 0;
    char diver_report[100];
    diver_report[0] = 0;

    for (int i = 0; i < DIVER_COUNT; i++) {
        if (divers_fishing[i]) {
            strcat(diver_report, "*");
            total_divers++;
        } else {
            strcat(diver_report, " ");
        }
    }

    // reef report
    char reef_report[100];
    reef_report[0] = 0;
    for (int i = 0; i < total_sharks; i++)
        strcat(reef_report, "+");
    for (int i = 0; i < total_divers; i++)
        strcat(reef_report, "*");
    for (int i = strlen(reef_report); i < MAX_IN_REEF; i++)
        strcat(reef_report, " ");

    printf("[%s] %s [%s]\n", shark_report, reef_report, diver_report);
    if (total_sharks > 0 && total_divers > 0)
        printf("!!! ERROR: diver getting eaten\n");

    fflush(stdout);
}

//I included a detailed breakdown of each line for better understanding of this implementation.
void *shark(void *arg) {
    // Convert the argument to an integer representing the shark's ID
    int k = *(int *)arg;

    // Infinite loop to continuously simulate shark behavior
    for (;;) {
        // Shark waits for a random amount of time before getting hungry
        // The waiting time is between 0 and SHARK_WAITING_TIME microseconds
        usleep(SHARK_WAITING_TIME);

        // Acquire the semaphore to enter the reef or wait if the reef is full
        sem_wait(&reef_semaphore);

        // Checking if any divers are currently in the reef
        if (waiting_divers == 0) {
            // Increment the number of sharks in the reef
            waiting_sharks++;
            // Set the shark's feeding status to true
            sharks_feeding[k] = true;
            // Call report to print the current state of the reef
            report();
            //How long the sharks are feeding in the reef
            usleep(SHARK_FISHING_TIME);

            // Set the shark's feeding status to false after feeding is done
            sharks_feeding[k] = false;
            // Decrement the number of sharks in the reef
            waiting_sharks--;
            // Call report again to update the current state of the reef
            report();
        }
        // Release the semaphore to allow others to enter the reef
        sem_post(&reef_semaphore);
    }
    return NULL;
}




void *diver(void *arg) {
    //See the shark function for a detailed breakdown of what happens in these functions ;)
    int k = *(int *)arg;

    for (;;) {
        usleep(DIVER_WAITING_TIME);

        sem_wait(&reef_semaphore);
        if (waiting_sharks == 0) {
            waiting_divers++;
            divers_fishing[k] = true;
            report();

            usleep(DIVER_FISHING_TIME);

            divers_fishing[k] = false;
            waiting_divers--;
            report();
        }
        sem_post(&reef_semaphore);
    }

    return NULL;
}


int main(void) {

    //random number generator initialization
    srandom(time(NULL));
    // initialize synchronization variables here
    int s = sem_init(&reef_semaphore, 0, MAX_IN_REEF);
    assert(s == 0);
    // end of synchronization variable initialization

    // initialize shared state
    sharks_feeding = malloc(sizeof(bool) * SHARK_COUNT);
    assert(sharks_feeding != NULL);
    for (int i = 0; i < SHARK_COUNT; i++)
        sharks_feeding[i] = false;

    divers_fishing = malloc(sizeof(bool) * SHARK_COUNT);
    assert(divers_fishing != NULL);
    for (int i = 0; i < DIVER_COUNT; i++)
        divers_fishing[i] = false;

    pthread_t sharks[SHARK_COUNT];
    pthread_t divers[DIVER_COUNT];

    // spawn the sharks
    int shark_counts[SHARK_COUNT];
    for (int i = 0; i < SHARK_COUNT; i++) {
        // create a new thread for this shark
        shark_counts[i] = i;
        int s = pthread_create(&sharks[i], NULL, shark, &shark_counts[i]);
        assert(s == 0);
        s = pthread_detach(sharks[i]);
        assert(s == 0);
    }

    // spawn the divers
    int diver_counts[DIVER_COUNT];
    for (int i = 0; i < DIVER_COUNT; i++) {
        // create a new thread for this diver
        diver_counts[i] = i;
        int s = pthread_create(&divers[i], NULL, diver, &diver_counts[i]);
        assert(s == 0);
        s = pthread_detach(divers[i]);
        assert(s == 0);
    }

    // let the simulation run for a while
    sleep(TOTAL_SECONDS);
    fflush(stdout);

    return 0;
}