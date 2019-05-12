//********************************************************************
// Mehmet Ozkan
// Advanced Operating Systems
// Project #2: Berkeley Algorithm
// 11/12/2018
// Instructor: Dr. Mohammed Belkhouche
//
//********************************************************************



#include <cmath>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <math.h>
#include <fstream>
#include <iostream>


using namespace std;


int main(int argc, char *argv[])
{
	int rank, num_of_proc, coordinator, index_of_timediff, coordinatordiff;
	int line_counter = 0;
	int adjusted_time,diff_ignore;
	int min_procs, coordinator_time_mins, diff_in_min, diff,  sum_of_check_diffs, diff_process_min[30], check_diff[30];
	int proc_hour, proc_mins;
    int joined_processes;
	float value, dummy_min;
	float average_diff=0, total_diff=0;
	
	string time_ignore;
	
	string line;
	
	struct 
	{
     int hour;
     int mins;
     
	}procs_time, coord_process;

	
	
	
	MPI_Datatype ptype;
	MPI_Status status;
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
	MPI_Comm_size(MPI_COMM_WORLD, &num_of_proc); 
	MPI_Type_contiguous(2,MPI_INT,&ptype);
	MPI_Type_commit(&ptype);
	
	
	if(argc !=2) // error checking if the  number of arguments are different than 2 
	{
		printf("[Error]:: Process[%d] could not read the file! Please type the correct number of arguments!\n", rank);
		MPI_Finalize();
		return 0;
	}
	
	 
	fstream my_file(argv[1], ios_base::in); // open the file from command line to read
	
	if(my_file == 0) // error checking if the file name is correct or not 
	{
			printf("[Error]:: Process[%d] could not read the file! Please type the correct name of the file!\n", rank);
			MPI_Finalize();
			return 0;
	}
	
	if (my_file.is_open())
	{
			while(getline(my_file,line))// store each line from the file into the array
		{
			
			
			
			if(line_counter==0)
			{
			coordinator = atoi(line.c_str()); // assign the coordinator process rank from the first line of the file
			
				if(rank == coordinator)
				{
				printf("I am process with rank %d acting as the coordinator process\n", rank);	
				}
			}
			
			if(rank == (line_counter-1))
			{
				
				
				procs_time.hour = ((line[0]-'0')*10) + (line[1]-'0'); // store time as hours
				procs_time.mins = ((line[3]-'0')*10) + (line[4]-'0'); // store time as minutes
				min_procs = ((procs_time.hour*60) + procs_time.mins ); // convert each process time to the minutes 
				
				if(procs_time.hour > 23 || procs_time.mins > 59 || procs_time.hour < 0 || procs_time.mins < 0)
				{
				printf("[Error]:: Process[%d] has received incorrect time value !\n", rank);	
				MPI_Abort(MPI_COMM_WORLD, 911);
				MPI_Finalize();
				return 0;
				
				}
			}
			
			
					
			if(line_counter==num_of_proc+1)
			{
				if(rank == coordinator)
				{
							
				int hour_ignore 	= ((line[0]-'0')*10) + (line[1]-'0'); // store time difference as hours
				int minute_ignore  = ((line[3]-'0')*10) + (line[4]-'0'); // store time difference as minutes
				diff_ignore = (hour_ignore*60) + minute_ignore; // convert time difference to the minutes 
							
				}	
				
			}
			
			
			line_counter++; //increase the line counter by 1
		}
		
				
			if(num_of_proc != line_counter-2) // error checking: number of processes should be equal to the number of process in txt file
			{
				printf("[Error]:: Number of processes must be %d\n", line_counter-2);
				MPI_Finalize();
				return 0;
			}
		
	}	
	
		MPI_Barrier(MPI_COMM_WORLD);
	
			
		if(rank == coordinator) // the coordinator process
		{
			
		int counter =0;
		coord_process.hour = procs_time.hour; // assign coordinator time as hour 
		coord_process.mins = procs_time.mins; // assign coordinator time as minute
		
		joined_processes = num_of_proc; // assign initially num_of_proc to the joined_processes for calculating average
		
		
		printf("Coordinator process is sending time %d:%d \n",coord_process.hour,coord_process.mins); //display the coordinator time 
		
	
		MPI_Bcast(&coord_process, 1, ptype, coordinator, MPI_COMM_WORLD); // broadcast the coordinator time
		MPI_Barrier(MPI_COMM_WORLD);
		
		
		
		coordinatordiff=0;//assign coordinator time difference is 0
		MPI_Send(&coordinatordiff, 1, MPI_INT, coordinator, 55, MPI_COMM_WORLD); // send time difference of the coordinator which is 0 to the coordinator
		
		
		
		
		for(int i=0; i<num_of_proc; i++)
		{
		  
			MPI_Recv(&diff,  1,  MPI_INT,  MPI_ANY_SOURCE, 55, MPI_COMM_WORLD, &status);
			
			diff_process_min[status.MPI_SOURCE] = diff;
			
			printf("Process %d has received time differential value of %d\n", rank, diff); //display the received time differential for each process
			
			
			if(abs(diff) > diff_ignore) // check if the time difference needs to be ignored 
			{
				counter = counter+1;
				check_diff[status.MPI_SOURCE] = 0; // assign check_diff as 0 if the difference is ignored 
				
				printf("Coordinator Process is ignoring time differential value of %d from process %d\n", diff, status.MPI_SOURCE);
			}
			
			else
			
				check_diff[status.MPI_SOURCE] = 1; // assign check_diff as 1 if the difference is not ignored
				
		}
		
	
		
		
		
		
		for(int i=0; i<num_of_proc; i++)
		{
			
		total_diff = total_diff + (diff_process_min[i]*check_diff[i]); // if check_diff[] is 0 then total difference is not changing 
																	  // but ignored process is removing for calculating average_diff
																	 // if check[] is 1 then total difference is changing by difference from processes 
																	// and the average_diff is updating 
		}								  
			
		average_diff = total_diff / (num_of_proc - counter); // average_diff value
		min_procs = min_procs + average_diff; // adjust coordinator time value
		
		printf("Time differential average_diff is %f\n", average_diff); // display differential average_diff
				
		}
		
		
			
		if(rank == coordinator) // the coordinator process
		{
		for(int i=0; i < num_of_proc; i++)
		{
			
				value = average_diff - diff_process_min[i]; // adjust time to be adjusted for each process
				printf("Coordinator process is sending the clock adjustment value of %f to process %d\n", value, i);
				MPI_Send(&value, 1, MPI_FLOAT, i, 33, MPI_COMM_WORLD); // send time to be adjusted to respective processes
			
		}
		
		
		
		}
	else
		{		
	
		MPI_Bcast(&coord_process, 1, ptype, coordinator, MPI_COMM_WORLD); // broadcast the coordinator time // receive the coordinator time
		MPI_Barrier(MPI_COMM_WORLD);
		printf("Process %d has received time %d:%d\n", rank, coord_process.hour,coord_process.mins); // display the received time for each processor 
		
			
		int coord_hour = coord_process.hour; // store the coordinator time as hours
		int coord_mins = coord_process.mins;// store the coordinator time as minutes
		coordinator_time_mins = (coord_hour*60) + coord_mins; // convert received coordinator time to the minutes 
		
		diff_in_min = min_procs - coordinator_time_mins; // calculate time difference between local process time and the coordinator time
		printf("Process %d is sending time differential value of %d to process %d\n", rank, diff_in_min, coordinator); 
		
		MPI_Send(&diff_in_min, 1, MPI_INT, coordinator, 55, MPI_COMM_WORLD); // send time difference to the coordinator
		MPI_Recv(&value, 1, MPI_FLOAT, MPI_ANY_SOURCE, 33, MPI_COMM_WORLD, &status); // receive time value which needs to be adjusted from the coordinator
		
		printf("Process %d has received the clock adjustment value of %f\n", rank, value); // display the adjustment value for each process
		
		min_procs = min_procs + value; // assign the adjusted time to the local time
		}
	
		int hour_adjusted = min_procs / 60;
		int min_adjusted = min_procs % 60;
		
		
		printf("Adjusted local time at process %d is %d:%d \n", rank, hour_adjusted, min_adjusted); // display adjusted local time for each process
		
	MPI_Finalize();
	return 0;
}