#include "types.h"
#include "stat.h"
#include "user.h"
#include "spinlock.h"

int array_sum(short* start_pos_array, short* last_pos_array) {
	int local_sum = 0;
	for (short* i=start_pos_array; i<last_pos_array; i++) {
		local_sum += (int) *i;
	}
	return local_sum;
}

float array_sqr_distance_from_mean(float mean, short* start_pos_array, short* last_pos_array) {
	float local_sum = 0.0;
	for (short* i=start_pos_array; i<last_pos_array; i++) {
		float fi = (float) *i;
	// printf(1, "here %d\n", (int) fi);
	// printf(1, "%d\n", (int) mean);
		float diff = mean-fi;
		local_sum += diff*diff;
	}
	return local_sum;
}

int
main(int argc, char *argv[])
{
	if(argc< 2){
		printf(1,"Need type and input filename\n");
		exit();
	}
	char *filename;
	filename=argv[2];
	int type = atoi(argv[1]);
	printf(1,"Type is %d and filename is %s\n",type, filename);

	int tot_sum = 0;	
	float variance = 0.0;

	int size=1000;
	short arr[size];
	char c;
	int fd = open(filename, 0);
	for(int i=0; i<size; i++){
		read(fd, &c, 1);
		arr[i]=c-'0';
		read(fd, &c, 1);
	}	
  	close(fd);
  	// this is to supress warning
  	printf(1,"first elem %d\n", arr[0]);
  
  	//----FILL THE CODE HERE for unicast sum and multicast variance

	int total_children = 7;
	int child_pid_arr[total_children];
	int children_spawned = 0;
	int parent_pid = getpid();
	short* start_pos;
	short* last_pos;

	int cid;
	while(children_spawned<total_children) {
		cid = fork();
		children_spawned++;
		if (cid==0) {
			start_pos = (short*) &arr;
			last_pos = (short*) &arr;
			start_pos += (children_spawned-1)*(size/total_children);
			if (children_spawned==total_children) {
				last_pos += size;
			} else {
				last_pos += (children_spawned)*(size/total_children);
			}
			int partial_sum = array_sum(start_pos, last_pos);
			int* msg = (int*) malloc(8);
			*msg = partial_sum;
			// printf(1, "%d\n", *((int*) msg));
			send(getpid(), parent_pid, msg);
			free(msg);
			break;
		}
		child_pid_arr[children_spawned-1] = cid;
	}

	float mean;
	if (cid!=0) {
		float* msg = (float*) malloc(8);
		for (int i=0; i<total_children; i++) {
		// printf(1, "%d\n", *((int*)msg));
			recv(msg);
			tot_sum += *((int*)msg);
		}
		if (type==1) {
			mean = (float) tot_sum;
			mean /= (float) size;
			*msg = mean;
			for (int i=0; i<total_children; i++) {
				send(parent_pid, child_pid_arr[i], msg);
			}
			for (int i=0; i<total_children; i++) {
				recv(msg);
				variance += *msg;	
			}
		variance /= (float) size; 
		}
		free(msg);
		for (int i=0; i<children_spawned; i++) {
		// printf(1, "%d\n", i);
			wait();
		}
	} else {
		if (type==1) {
			float* msg = (float*) malloc(8);
			recv(msg);
			mean = *msg;
			// printf(1, "%d\n", mean);
			// printf(1, "%s\n", *((int*) msg));
			// sleep(10*getpid());
			float sqr_sum = array_sqr_distance_from_mean(mean, start_pos, last_pos);
			*msg = sqr_sum;
			send(getpid(), parent_pid, msg);
			free(msg);
		}
		// printf(1, "%d\n", getpid());
		exit();
	}

  	//------------------

  	if(type==0){ //unicast sum
		printf(1,"Sum of array for file %s is %d\n", filename,tot_sum);
	}
	else{ //mulicast variance
	// printf(1, "parent here %d\n", variance);
	// int var = (int) variance;
	// printf(1, "parent here %d\n", var);
		printf(1,"Variance of array for file %s is %d\n", filename,(int) variance);
	}
	exit();
}
