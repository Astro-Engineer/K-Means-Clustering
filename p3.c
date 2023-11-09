#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define h 800
#define w 800
#define NUM_ITERATIONS 50
#define INPUT_FILE "input.raw"
#define OUTPUT_FILE "output.raw"
const int NUM_CLUSTERS = 6;

struct ThreadData {
    unsigned char *image_data;
    int start, end;
    float *cluster_means;
    int *cluster_ids;
    int thread_id;
    int num_threads;
};

int r;
pthread_mutex_t lockVar;
pthread_cond_t threadCond;

//find distance from 2 values
float distanceCalc(unsigned char val, float mean) {
    return fabs((float)val - mean);
}

//find closest cluster
int clusterSearch(float *distances, int count) {
    int minIdx = 0;
    for (int i = 1; i < count; i++) {
        if (distances[i] < distances[minIdx]) minIdx = i;
    }
    return minIdx;
}

//set initial values
void setMeans(float means[]) {
    means[0] = 0.0;
    means[1] = 65.0;
    means[2] = 100.0;
    means[3] = 125.0;
    means[4] = 190.0;
    means[5] = 255.0;
}

void *kMeansClustering(void *arg) {
    struct ThreadData *data = (struct ThreadData *)arg;
    unsigned char *image_data = data->image_data;
    float distances[NUM_CLUSTERS];
    int cluster_counts[NUM_CLUSTERS];
    float cluster_sums[NUM_CLUSTERS];
    float *cluster_means = (float *)malloc(NUM_CLUSTERS * sizeof(float));

    setMeans(cluster_means);
    data->cluster_means = cluster_means;

    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        //record start time
        struct timespec start_time, end_time;
        clock_gettime(CLOCK_MONOTONIC, &start_time);
        //prepare memory for updating data
        memset(cluster_counts, 0, sizeof(cluster_counts));
        memset(cluster_sums, 0, sizeof(cluster_sums));

        for (int i = data->start; i <= data->end; i++) {
            for (int j = 0; j < NUM_CLUSTERS; j++) {
                //find min distance between image data and cluster mean
                distances[j] = distanceCalc(image_data[i], cluster_means[j]);
            }
            int clusterIdx = clusterSearch(distances, NUM_CLUSTERS);
            data->cluster_ids[i] = clusterIdx;

            cluster_sums[clusterIdx] += image_data[i];
            cluster_counts[clusterIdx]++;
        }

        //record end time
        clock_gettime(CLOCK_MONOTONIC, &end_time);

        //calculate time taken 
        double elapsed_time = (end_time.tv_sec - start_time.tv_sec) +
                              (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
        printf("Thread %d - Iteration %d: Time taken = %f seconds\n",
               data->thread_id, iter, elapsed_time);
        //lock to ensure no race condition
        pthread_mutex_lock(&lockVar);
        if (r < data->num_threads - 1) {
            r++;
            pthread_cond_wait(&threadCond, &lockVar);
        } else {
            for (int m = 0; m < NUM_CLUSTERS; m++) {
                cluster_means[m] = cluster_counts[m] ? cluster_sums[m] / cluster_counts[m]
                                            : cluster_means[m];
            }
            r = 0;
            pthread_cond_broadcast(&threadCond);
        }
        pthread_mutex_unlock(&lockVar);
    }
    return NULL;
}

void performKMeans(int num_threads) {
    //max number of threads
    pthread_t threads[8];
    //allocate data for the image
    unsigned char *image_data =
        (unsigned char *)malloc(h * w * sizeof(unsigned char));
    int *cluster_ids = (int *)malloc(h * w * sizeof(int));
    float *global_cluster_means = (float *)malloc(NUM_CLUSTERS * sizeof(float));
    int pixels_per_thread = (w * h) / num_threads;
    struct timespec start, stop;
    double execution_time;

    FILE *file_ptr = fopen(INPUT_FILE, "rb");
    if (!file_ptr) {
        printf("Cannot open file\n");
        return;
    }
    fread(image_data, sizeof(unsigned char), w * h, file_ptr);
    fclose(file_ptr);

    if (clock_gettime(CLOCK_REALTIME, &start) == -1) {
        perror("clock gettime");
    }

    pthread_mutex_init(&lockVar, NULL);
    pthread_cond_init(&threadCond, NULL);
    //create thread data array (maxes at 8)
    struct ThreadData thread_data_array[8];

    for (int i = 0; i < num_threads; i++) {
        thread_data_array[i].image_data = image_data;
        thread_data_array[i].start = i * pixels_per_thread;
        thread_data_array[i].end = (i + 1) * pixels_per_thread - 1;
        thread_data_array[i].cluster_ids = cluster_ids;
        thread_data_array[i].thread_id = i;
        thread_data_array[i].num_threads = num_threads;

        pthread_create(&threads[i], NULL, kMeansClustering, &thread_data_array[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    memset(global_cluster_means, 0, NUM_CLUSTERS * sizeof(float));
    for (int i = 0; i < num_threads; i++) {
        for (int j = 0; j < NUM_CLUSTERS; j++) {
            global_cluster_means[j] += thread_data_array[i].cluster_means[j];
        }
        free(thread_data_array[i].cluster_means);
    }

    for (int i = 0; i < NUM_CLUSTERS; i++) {
        global_cluster_means[i] /= num_threads;
    }

    for (int i = 0; i < w * h; i++) {
        image_data[i] = (unsigned char)global_cluster_means[cluster_ids[i]];
    }

    clock_gettime(CLOCK_REALTIME, &stop);
    if (clock_gettime(CLOCK_REALTIME, &stop) == -1) {
        perror("clock gettime");
    }
    execution_time = (stop.tv_sec - start.tv_sec) +
                     (double)(stop.tv_nsec - start.tv_nsec) / 1e9;
    printf("Execution time = %f sec\n", execution_time);

    file_ptr = fopen(OUTPUT_FILE, "wb");
    if (!file_ptr) {
        printf("Cannot open file\n");
        return;
    }
    fwrite(image_data, sizeof(unsigned char), w * h, file_ptr);
    fclose(file_ptr);

    //free memory
    free(image_data);
    free(cluster_ids);
    free(global_cluster_means);
    //destory mutex variables
    pthread_mutex_destroy(&lockVar);
    pthread_cond_destroy(&threadCond);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <num_threads_integer>\n", argv[0]);
        return 1;
    }
    //take input of threads and run function
    int num_threads = atoi(argv[1]);
    performKMeans(num_threads);

    return 0;
}