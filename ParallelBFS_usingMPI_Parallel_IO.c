#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <stdbool.h>

/*
 * This method checks the frontier vector F for all 0's to indicate entire graph has traversed.
*/
bool isClear(int[], long int, long int,long int );

/*
 * Main start of the program
 * Denotions used in the below code:
 *	F -> Frontier vector to identify nodes to be traversed in next level.
 * 	T -> Temporary vector to hold the next level nodes.
 * 	P -> Parent vector used to hold the nodes already visited.
 *	Aij -> Adjacency matrix local to each processor.
*/
int main (int argc, char *argv[])
{
    unsigned long long int rowNo, columnNo;
    unsigned long long int noofPRows, noofVertices, noofVerticesPerProcessor;
    double t1, t2;
    int rank, numtasks, size;
    MPI_Init(&argc, &argv);
    t1 = MPI_Wtime(); 
	
      MPI_Status status;
      MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
      MPI_Comm_rank(MPI_COMM_WORLD,&rank);
      noofPRows = (int) sqrt(numtasks);
      noofVertices = atoll(argv[2]);
      noofVerticesPerProcessor = (int) ceil(atoi(argv[2])/(noofPRows*noofPRows));
	
      unsigned long long int i,j,k;
      int* F;
      F = (int *) malloc(sizeof(int)*noofVertices);
      for(i=0; i<noofVertices; i++){
		F[i] = 0;
      }
      F[atoi(argv[4])] = 1;
      int* Fij;
      Fij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
      int* Pij;
      Pij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
      
      int* Tij;
      Tij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
      for( i =0; i<noofVerticesPerProcessor; i++){
		Pij[i] = 0;
		Fij[i] = 0;
		Tij[i] = 0;
      }
      
      int* Ti;
      Ti = (int *) malloc(sizeof(int)*noofVerticesPerProcessor*noofPRows);
      for(i=0; i < noofVerticesPerProcessor*noofPRows ; i++){
		Ti[i] = 0;
      }
      
      unsigned long long int noofVerticesinRowofProcessor = noofVerticesPerProcessor*noofPRows;
      
      char* Aij;
      Aij = (char *) malloc(sizeof(char)*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor);
      
      for(i=0; i < noofVerticesinRowofProcessor*noofVerticesinRowofProcessor ; i++){
		Aij[i] = 0;
      }
          
    //Reading File.
      MPI_Comm rowComm, colComm;
      rowNo = (rank)/noofPRows + 1;
      columnNo = rank%noofPRows +1;
      
      int i1,j1; 
      
      i = rank/noofPRows;
      j = rank%noofPRows;

      MPI_Status stat;
      MPI_Datatype MPI_CHUNK;
      MPI_Type_vector(noofVerticesinRowofProcessor,noofVerticesinRowofProcessor,noofVertices,MPI_CHAR,&MPI_CHUNK);
      MPI_Type_commit(&MPI_CHUNK);
      MPI_File fh;
      MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
      MPI_Offset offset = (i*noofVertices*noofVerticesinRowofProcessor+j*noofVerticesinRowofProcessor) * sizeof(char) ;
      MPI_File_set_view(fh, offset, MPI_CHAR, MPI_CHUNK, "native", MPI_INFO_NULL);
       
      MPI_File_read(fh, Aij,1ULL*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor, MPI_CHAR, &stat);
      MPI_Type_free(&MPI_CHUNK);
      MPI_File_close(&fh);
	        
      MPI_Comm_split(MPI_COMM_WORLD, columnNo, rank, &colComm); 
      MPI_Comm_split(MPI_COMM_WORLD, rowNo, rank, &rowComm);

      
	for(i=0; i< noofVerticesPerProcessor; i++){
		Fij[i] = F[(rowNo-1)*noofVerticesPerProcessor*noofPRows+(columnNo-1)*noofVerticesPerProcessor +i];
        Pij[i] = Fij[i];
    }
      int* rec_buffer = (int*)malloc(sizeof(int)*noofVerticesinRowofProcessor);
      int* send_buffer = (int*)malloc(sizeof(int)*noofVerticesPerProcessor);      

      t2 = MPI_Wtime(); 
      if(rank==0)
      	printf( "Elapsed time for data reading and initialization %f\n", t2 - t1 ); 
        int roundNo=1;
        int noofvertices=0;
    while(isClear(F,rowNo, columnNo, noofVertices)) {
		for(i=0; i<noofVerticesPerProcessor; i++){
             send_buffer[i] = Fij[i];
          }
         
		if(rowNo != columnNo) {
			if(rowNo > columnNo) {
				MPI_Send(send_buffer, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD);
				MPI_Recv(Fij, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD, &status);
			} else {
				MPI_Recv(Fij, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD, &status);
				MPI_Send(send_buffer, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD); 
			}
		}
         MPI_Allgather(Fij,noofVerticesPerProcessor,MPI_INT,rec_buffer,noofVerticesPerProcessor,MPI_INT,colComm);
           
         int val=0;
         
         for(i=0; i<noofVerticesinRowofProcessor; i++){
             val=0;
             for(j=0;j<noofVerticesinRowofProcessor; j++){
                val += Aij[i*noofVerticesinRowofProcessor + j]*rec_buffer[j];
	     }
             Ti[i] = val;
         }

         MPI_Alltoall(Ti,noofVerticesPerProcessor,MPI_INT,rec_buffer,noofVerticesPerProcessor,MPI_INT, rowComm);
		 for(i=0; i<noofPRows; i++){
           for(j=0; j<noofVerticesPerProcessor;j++){
             if(rec_buffer[i*noofVerticesPerProcessor+j] > 0){
                Tij[j]=1;
             }
           }
         }
        
         for(i=0; i<noofVerticesPerProcessor; i++){
             if(Pij[i] == 1 && Tij[i]==1){
                Tij[i]=0;
             }
         }
         for(i=0; i<noofVerticesPerProcessor; i++){
             if(Pij[i] == 0 && Tij[i]==1){
                Pij[i]=1;  
             }
         }
         for(i=0; i<noofVerticesPerProcessor; i++){
             Fij[i] = Tij[i];
         }
  
         MPI_Allgather(Fij,noofVerticesPerProcessor,MPI_INT,rec_buffer,noofVerticesPerProcessor,MPI_INT,rowComm);
         
         MPI_Allgather(rec_buffer,noofVerticesinRowofProcessor,MPI_INT,F,noofVerticesinRowofProcessor,MPI_INT,colComm);
         
       }
      
    t1 = MPI_Wtime(); 
   fflush(stdout);
    MPI_Finalize();
}

/*
 * This method checks the frontier vector F for all 0's to indicate entire graph has traversed.
 * If any un-traversed node has been found i.e if any value in F vector equals 1, then this function returns true.
 * Else, function returns false.
*/
bool isClear(int F[],long int rowNo,long int columnNo,long int size) {
  int temp,i;
  for(temp = 0;temp<size;temp++) {
    if(F[temp]==1) {
       return true;
    }
  }
  return false;
}
