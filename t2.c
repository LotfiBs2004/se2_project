#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

typedef struct {
    pthread_mutex_t mutex;
    char direction;
    int count_x;
    int count_y;
    int waiting_x;
    int waiting_y;
    sem_t x_sem;
    sem_t y_sem;
} Tunnel;

typedef struct {
    int bus_id;
    char city;
    Tunnel* tunnel;
} BusArgs;

// Add a global mutex for thread-safe printing
pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_tunnel(Tunnel* tunnel) {
    pthread_mutex_init(&tunnel->mutex, NULL);
    tunnel->direction = '\0';
    tunnel->count_x = 0;
    tunnel->count_y = 0;
    tunnel->waiting_x = 0;
    tunnel->waiting_y = 0;
    sem_init(&tunnel->x_sem, 0, 0);
    sem_init(&tunnel->y_sem, 0, 0);
}

void destroy_tunnel(Tunnel* tunnel) {
    pthread_mutex_destroy(&tunnel->mutex);
    sem_destroy(&tunnel->x_sem);
    sem_destroy(&tunnel->y_sem);
}

// ... (enter_from_x, exit_from_x, enter_from_y, exit_from_y functions remain unchanged) ...

void* bus_behavior(void* arg) {
    BusArgs* args = (BusArgs*)arg;
    int bus_id = args->bus_id;
    char city = args->city;
    Tunnel* tunnel = args->tunnel;
    free(arg);

    for (int i = 0; i < 10; i++) {
        // Outbound trip
        char dest = (city == 'X') ? 'Y' : 'X';
        
        // Protect printf with mutex
        pthread_mutex_lock(&print_mutex);
        printf("[%c->%c] Bus %d : traversée %d\n", city, dest, bus_id, i+1);
        pthread_mutex_unlock(&print_mutex);

        if (city == 'X') enter_from_x(tunnel);
        else enter_from_y(tunnel);

        usleep(1000000 + rand() % 500000); // Simulate travel

        if (city == 'X') exit_from_x(tunnel);
        else exit_from_y(tunnel);

        // Return trip
        pthread_mutex_lock(&print_mutex);
        printf("[%c->%c] Bus %d : traversée %d\n", dest, city, bus_id, i+1);
        pthread_mutex_unlock(&print_mutex);

        if (dest == 'X') enter_from_x(tunnel); // Enter from return direction
        else enter_from_y(tunnel);

        usleep(1000000 + rand() % 500000);

        if (dest == 'X') exit_from_x(tunnel);
        else exit_from_y(tunnel);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    Tunnel tunnel;
    init_tunnel(&tunnel);
    pthread_t threads[9];

    // Create buses (5 from X, 4 from Y)
    for (int i = 0; i < 9; i++) {
        BusArgs* args = malloc(sizeof(BusArgs));
        args->bus_id = (i < 5) ? i+1 : i-4; // IDs 1-5 for X, 1-4 for Y
        args->city = (i < 5) ? 'X' : 'Y';
        args->tunnel = &tunnel;
        pthread_create(&threads[i], NULL, bus_behavior, args);
    }

    for (int i = 0; i < 9; i++) pthread_join(threads[i], NULL);
    destroy_tunnel(&tunnel);
    pthread_mutex_destroy(&print_mutex);
    return 0;
}