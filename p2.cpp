#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>
#include <algorithm>
#include <cmath>

#define h  800 
#define w  800

#define input_file  "input.raw"
#define output_file "output.raw"
int NUM_THREADS = 0;
unsigned char *a;
std::vector<double> meanValues = {0,65,100,125,190,255};
struct  thread_data{
    int start = 0;
    int end = 0;    
    std::vector<double> clusterSum {0,0,0,0,0,0};
    std::vector<double> clusterLocation {0,0,0,0,0,0};
};

std::vector<double> totalSum = {0,0,0,0,0,0};
std::vector<double> totalLocation = {0,0,0,0,0,0};

void *clusterLogic(void *threadarg){
	thread_data* my_data = static_cast<thread_data*>(threadarg);
    int start = my_data->start;
    int end = my_data->end;
    std::vector<double> clusterSumTemp = my_data->clusterSum;
    std::vector<double> clusterLocationTemp = my_data->clusterLocation;
    std::vector<double> distance = {0,0,0,0,0,0};
    for (int i = start; i < end; i++){
        for (int j = 0; j < 6; j++){ 
            distance[j] = fabs(meanValues[j] - static_cast<double>(a[i])); 
        }

		double minimum = distance[0];
		for (int k = 0; k < 6; k++){
            if (distance[k] < minimum){ 
                minimum = distance[k];
            }
        }

		for (int x = 0; x < 6; x++){
			if (distance[x] == minimum){
				my_data->clusterSum[x] += static_cast<double>(a[i]);
				my_data->clusterLocation[x]++;
			}
		}
    }

    pthread_exit(NULL);
};

int main(int argc, char** argv){
    int i;
    FILE *fp;
    NUM_THREADS = std::stoi(argv[1]);
    struct  thread_data  thread_data_array[NUM_THREADS];
    pthread_t  threads[NUM_THREADS];
    pthread_attr_t attr;
	int rc;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    std::vector<double> distanceMain {0,0,0,0,0,0};
    std::vector<double> clustersMain {0,0,0,0,0,0};
    std::vector<double> sumMain {0,0,0,0,0,0};
  	a = new unsigned char[h*w];
    
	// the matrix is stored in a linear array in row major fashion
	if (!(fp=fopen(input_file, "rb"))) {
		std::cerr <<"can not opern file\n";
		return 1;
	}
	fread(a, sizeof(unsigned char), w*h, fp);
	fclose(fp);

    //verifies that the array a is full of doubles
    /*for(int num=0;num<800*200;num++){
        std::cout<<static_cast<double>(a[num])<<std::endl;
    }
    */
    struct timespec start, stop; 
    double time;
    
    double totalTime;
    if( clock_gettime(CLOCK_REALTIME, &start) == -1) { std::cerr<<"clock gettime";}
    for(int loops = 0;loops<50;loops++){
	// measure the start time here

    for(int i=0;i<NUM_THREADS;i++){
            thread_data_array[i].start = i * h/NUM_THREADS;
            thread_data_array[i].end =  (i+1) * h/NUM_THREADS;
            rc = pthread_create(&threads[i], &attr, clusterLogic, (void *) &thread_data_array[i]);
				if (rc) { printf("ERROR; return code from pthread_create() is %d\n", rc); exit(-1);}
    }
    
    for(int i=0;i<NUM_THREADS;i++){
            rc = pthread_join(threads[i], NULL);
            if (rc) { printf(" joining error %d ", rc); exit(-1);}
    }
    for(int j =0;j<NUM_THREADS;j++){
        for(int k = 0;k<6;k++){
            sumMain[k] = thread_data_array[j].clusterSum[k];
            clustersMain[k] = thread_data_array[j].clusterLocation[k];
        }
    }
    for (int k = 0; k < 6; k++){
        if (clustersMain[k] != 0){ 
            meanValues[k] = sumMain[k] / clustersMain[k]; 
        }
    }

	// measure the end time here
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { std::cerr<<"clock gettime";}	
    time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
    totalTime += time;
	// print out the execution time here
	std::cout<<"Execution time for loop:"<< loops<< " = "<< time<< " sec"<<std::endl;
    } 
    std::cout<<"Execution time for all loops:"<< " = "<< totalTime<< " sec"<<std::endl;
    double temp, cluster;
    for(i=0;i<h*w;i++){
            temp = abs(static_cast<double>(a[i]) - meanValues[0]);
            cluster = 0;
            if(abs(static_cast<double>(a[i]) - meanValues[1]) < temp){
                temp = abs(static_cast<double>(a[i]) - meanValues[1]);
                cluster = 1;
            }
            if(abs(static_cast<double>(a[i]) - meanValues[2]) < temp){
                temp = abs(static_cast<double>(a[i]) - meanValues[2]);
                cluster = 2;
            }
            if(abs(static_cast<double>(a[i]) - meanValues[3]) < temp){
                temp = abs(static_cast<double>(a[i]) - meanValues[3]);
                cluster = 3;
            }
            if(abs(static_cast<double>(a[i]) - meanValues[4]) < temp){
                temp = abs(static_cast<double>(a[i]) - meanValues[2]);
                cluster = 4;
            }
            if(abs(static_cast<double>(a[i]) - meanValues[5]) < temp){
                temp = abs(static_cast<double>(a[i]) - meanValues[3]);
                cluster = 5;
            }

            if(cluster == 0){
                a[i] = static_cast<unsigned char>(meanValues[0]);
            }
            else if(cluster == 1){
                a[i] = static_cast<unsigned char>(meanValues[1]);
            }
            else if(cluster == 2){
                a[i] = static_cast<unsigned char>(meanValues[2]);
            }
            else if(cluster == 3){
                a[i] = static_cast<unsigned char>(meanValues[3]);
            }
            else if(cluster == 4){
                a[i] = static_cast<unsigned char>(meanValues[4]);
            }
            else if(cluster == 5){
                a[i] = static_cast<unsigned char>(meanValues[5]);
            }

    }
	
	if (!(fp=fopen(output_file,"wb"))) {
		std::cerr << "can not opern file\n";
		return 1;
	}	
	fwrite(a, sizeof(unsigned char),w*h, fp);
    fclose(fp);

    delete[] a;
    
    return 0;
}