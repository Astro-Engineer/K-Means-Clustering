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

int main(int argc, char** argv){
    int i;
    FILE *fp;

  	unsigned char *a = new unsigned char[h*w];
    
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
    
    std::vector<double> meanValues = {0,85,170,255};
    double totalTime;

    for(int loops = 0;loops<30;loops++){
	// measure the start time here
    struct timespec start, stop; 
    double time;

    if( clock_gettime(CLOCK_REALTIME, &start) == -1) { std::cerr<<"clock gettime";}
		
        std::vector<double> cluster0 = {0,0};
        std::vector<double> cluster1 = {0,0};
        std::vector<double> cluster2 = {0,0};
        std::vector<double> cluster3 = {0,0};
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
            
            if(cluster == 0){
                cluster0[0] += static_cast<double>(a[i]);
                cluster0[1] = cluster0[1] + 1;
            }
            else if(cluster == 1){
                cluster1[0] += static_cast<double>(a[i]);
                cluster1[1] = cluster1[1] + 1;
            }
            else if(cluster == 2){
                cluster2[0] += static_cast<double>(a[i]);
                cluster2[1] = cluster2[1] + 1;
            }
            else if(cluster == 3){
                cluster3[0] += static_cast<double>(a[i]);
                cluster3[1] = cluster3[1] + 1;
            }
        }
        if(cluster0[0]==0){
            meanValues[0] = 0;
        }
        else{
            meanValues[0] = cluster0[0] / cluster0[1];
        }
        if(cluster1[0]==0){
            meanValues[1] = 0;
        }
        else{
            meanValues[1] = cluster1[0] / cluster1[1];
        }
        if(cluster2[0]==0){
            meanValues[2] = 0;
        }
        else{
            meanValues[2] = cluster2[0] / cluster2[1];
        }
        if(cluster3[0]==0){
            meanValues[3] = 0;
        }
        else{
            meanValues[3] = cluster3[0] / cluster3[1];
        }
		
	//
	
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
