#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include "mpi.h"
#define NUM_EDGES 2

int main(int argc, char **argv ) {
	
	char debug_buffer[1024];
	char tmp_str[100];
	
	int jj, ee, n, rank, size, rem, proc_down, proc_up;
	int* matrix;
	int el_each;
	int* my_space;
	int* num_local_row;
	int MY_NUM_OF_ROWS;
	int* EDGE1;
	int* EDGE2;
	int* CONTENT;
	
	n = atoi(argv[1]);

	MPI_Init(&argc, &argv);
	MPI_Status status;
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	
	num_local_row = (int*) malloc(size*sizeof(int));
	rem = n % size;
	//strcpy(debug_buffer," ");
	for (jj = 0; jj < size; jj++){
		num_local_row[jj] = (n - rem)/size;
		if(jj < rem){
			num_local_row[jj] += 1;
		}
		//sprintf(tmp_str, "%d, ",num_local_row[jj] );
		//strcat(debug_buffer,tmp_str);
	}
	MY_NUM_OF_ROWS = num_local_row[rank];
	
	my_space = (int*) malloc(((MY_NUM_OF_ROWS+NUM_EDGES)*n)*sizeof(int));
	EDGE1 = &my_space[0];
	EDGE2 = &my_space[(MY_NUM_OF_ROWS+1)*n];
	CONTENT = &my_space[n];
	
	//printf("[rank %d] n = %d, my num of rows = %d, my space = %p, edge 1 = %p, edge 2 = %p, content = %p \n",rank,n,num_local_row[rank],my_space,EDGE1,EDGE2,CONTENT);
	//printf("[rank %d] %s\n", rank, debug_buffer);
	
	if (rank == 0){
		int i, j, k;
		int rows_sent;
		
		FILE * fp;
		char * line=NULL;
		size_t len = 0;
		ssize_t read;
		
		matrix = (int*) malloc(n*n*sizeof(int));
		
		// File input #####################
		fp = fopen(argv[2],"r");
		if (fp == NULL){
			printf("No such file\n");
			exit(-1);
		}
		j = 0;
		while ((read = getline(&line,&len,fp)) != -1){
			matrix[j] = atoi(line);
			j++;
		}
		free(line);
		fclose(fp);
		// File input #####################
		//printf("File red\n");
		
		rows_sent = MY_NUM_OF_ROWS;
		for (k = 1; k < size; k++){
			//printf("rows sent = %d\n",rows_sent);
			int* split = &matrix[rows_sent*n];
			MPI_Send(split,num_local_row[k]*n,MPI_INT,k,10,MPI_COMM_WORLD);
			rows_sent = rows_sent + num_local_row[k];
		}
		
		// Fake send a me stesso (rank = 0), mi tengo le prime righe
		for (i = 0; i < num_local_row[rank]*n; i++){
			CONTENT[i] = matrix[i];
		}
		free(matrix);
	}
	
	if (rank != 0){
		MPI_Recv(CONTENT, MY_NUM_OF_ROWS*n, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
	}
	
	//MPI_Send(CONTENT,n,MPI_INT,(rank-1)%size,10,MPI_COMM_WORLD);
	//MPI_Recv(EDGE2,n,MPI_INT,(rank+1)%size,10,MPI_COMM_WORLD,&status);
	
	proc_down = rank + 1;
	proc_up = rank - 1;
	if (rank == size-1){
		proc_down = 0;
	}
	if (rank == 0){
		proc_up = size-1;
	}
	
	//printf("[rank %d] rank down = %d, rank up = %d\n", rank,proc_down,proc_up);
	
	MPI_Sendrecv(&CONTENT[(MY_NUM_OF_ROWS-1)*n],n,MPI_INT,proc_down,10,EDGE1,n,MPI_INT,proc_up,10,MPI_COMM_WORLD,&status);
	MPI_Sendrecv(CONTENT,n,MPI_INT,proc_up,10,EDGE2,n,MPI_INT,proc_down,10,MPI_COMM_WORLD,&status);
	
	strcpy(debug_buffer," ");
	for (ee = 0; ee < (MY_NUM_OF_ROWS+NUM_EDGES)*n; ee++){
		sprintf(tmp_str, "%d, ",my_space[ee]);
		strcat(debug_buffer,tmp_str);
	}
	printf("[rank %d] my matrix is %s \n", rank,debug_buffer);
	
	free(my_space);
	MPI_Finalize();
	return(0);
}