#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <stdbool.h>
bool isClear(int[], long int, long int,long int );
long long int getIndex(long long int i,long long int j ,long long int rowSize);

int main (int argc, char *argv[])
{
    unsigned long long int rowNo, columnNo; // rowNo and columnNo of processor in the 2-D partition.
    unsigned long long int noofPRows;       // Number of processor rows in the 2-D partition.
    unsigned long long int noofVertices;    // Given input vertices count of the graph.
    unsigned long long int noofVerticesPerProcessor; // Number of vertices a processor own after 2-D patitioning.(size of distributed frontier vector)
    unsigned long long int NVertices; // Normalized vertices count for distributing input matrix uniformly across processors
    int rank, numtasks;              // rank of the processor in MPI_COMM_WORLD.
    
    MPI_Init(&argc, &argv);          // Start of parallel execution
    double t1, t2;                   // used to track execution time 
    t1 = MPI_Wtime();                // start of data setup phase
    MPI_Status status;
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    noofPRows = (int) sqrt(numtasks);        // calcuting no of processor rows in 2-D partition, this is also equal to processor columns.
    noofVertices = atoll(argv[1]);           // Size of the input graph (count of vertices)
    NVertices = ceil(noofVertices*1.0/numtasks)* numtasks; // normalizing the input vertices count for equal adjacency matrix distribution across processors.
      
    int dens = atoi(argv[3]);   // desired density of the graph to be generated
    noofVerticesPerProcessor = (int) ceil(NVertices/(noofPRows*noofPRows));// no of vertices in sub adjacency matrix stored at each processor.
    
    /* Intializing all variables for graph traversal */
    unsigned long long int i,j,k;
    int* F;                    //Global Frontier Vector.
    F = (int *) malloc(sizeof(int)*NVertices);
    for(i=0; i<NVertices; i++){
	F[i] = 0;
    }
    F[atoi(argv[2])] = 1;
    int* Fij;                 // Current Local frontier Vector.
    Fij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
    int* Pij;
    Pij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
    
    int* Tij;               // Next Local frontier Vector.
    Tij = (int *) malloc(sizeof(int)*noofVerticesPerProcessor);
    for( i =0; i<noofVerticesPerProcessor; i++){
	Pij[i] = 0;
	Fij[i] = 0;
	Tij[i] = 0;
    }
      
    int* Ti;                // Next frontier of a row of processors
    Ti = (int *) malloc(sizeof(int)*noofVerticesPerProcessor*noofPRows);
    for(i=0; i < noofVerticesPerProcessor*noofPRows ; i++){
        Ti[i] = 0;
    }
      
    unsigned long long int noofVerticesinRowofProcessor = noofVerticesPerProcessor*noofPRows;
    char* Aij;              // Adjacency matrix stored at each processor.
    Aij = (char *) malloc(sizeof(char)*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor);
      
    for(i=0; i < noofVerticesinRowofProcessor*noofVerticesinRowofProcessor ; i++){
        Aij[i] = 0;
    }
      
    if(rank==1){
	printf("noofVertices=%lld,nVertices=%lld, noofVerticesPerProcessor=%lld,Ti_size=%lld \n ",noofVertices,NVertices, noofVerticesPerProcessor,noofVerticesPerProcessor*noofPRows);
    }

    MPI_Comm rowComm, colComm;        // row communication world and column communication world
    rowNo = (rank)/noofPRows + 1;
    columnNo = rank%noofPRows +1;
    i = rank/noofPRows;
    j = rank%noofPRows;
      
    /* Random Graph generation with given Density as input.  */
    srand(100*rank);
    long long int noofOnes=0;
    double density=0;

    long long int i2,j2,k2;
    if(columnNo >= rowNo){
        for(i2=0; i2< noofVerticesinRowofProcessor*noofVerticesinRowofProcessor; i2++){
            int rand1 = rand()%100;
            if(rand1<dens){
                Aij[i2] = 1;
                noofOnes++;
            }
         }
      }
    density = 2*noofOnes*1.0/(noofVerticesinRowofProcessor*noofVerticesinRowofProcessor);
    if(rowNo != columnNo)
        noofOnes *= 2;
    int recv_ddata;  
    MPI_Reduce(&noofOnes, &recv_ddata,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    if(rank==0){
        printf("sum=%d, density=%f\n",recv_ddata,recv_ddata*1.0/(noofVertices*noofVertices));
    }
    
    /* Sending matrix data to processor below the diagonal, and also recieving data if a processor is below diagonal */
    char temp;
    long long int chunkSize = 214748364;
    long int  noofChunkstoSend = ceil(((noofVerticesinRowofProcessor*noofVerticesinRowofProcessor)*1.0)/(chunkSize*1.0));
    char* Aijtemp = Aij;
    int r;
    if(rowNo < columnNo) {
        for(r=0;r<noofChunkstoSend;r++){
            long long int size = chunkSize;
            if(r==noofChunkstoSend-1){
                size = noofVerticesinRowofProcessor*noofVerticesinRowofProcessor - r*chunkSize;
            }
            printf("sending %lld bytes in round %d \n",size,r+1);
            MPI_Send(Aijtemp, size, MPI_CHAR, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD);
            Aijtemp += size;
        }
    } else if(rowNo > columnNo) {
        for(r=0;r<noofChunkstoSend;r++){
            long long int size = chunkSize;
            if(r==noofChunkstoSend-1){
                size = noofVerticesinRowofProcessor*noofVerticesinRowofProcessor - r*chunkSize;
            }
        MPI_Recv(Aijtemp, size, MPI_CHAR, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD, &status);
        Aijtemp +=size;
    }
    for(i2=0;i2<noofVerticesinRowofProcessor;i2++) {
        for(j2=0;j2<noofVerticesinRowofProcessor;j2++){
            if(i2<j2){
                temp = Aij[getIndex(i2,j2,noofVerticesinRowofProcessor)];
                Aij[getIndex(i2,j2,noofVerticesinRowofProcessor)] = Aij[getIndex(j2,i2,noofVerticesinRowofProcessor)];
                Aij[getIndex(j2,i2,noofVerticesinRowofProcessor)] = temp;
             }
         }
     }
       }
    // Padding zero to normalize, if input vertices count makes non-uniform distribution.
    if(j==noofPRows-1){
        for (i=0;i<noofVerticesinRowofProcessor;i++) {
            for (j=(noofVerticesinRowofProcessor -(NVertices-noofVertices));j<noofVerticesinRowofProcessor;j++){
               Aij[i*noofVerticesinRowofProcessor+j] = 0;
            }
        }
    }       

    if(i==noofPRows-1){
        for (j=0;j<noofVerticesinRowofProcessor;j++) {
            for (i=(noofVerticesinRowofProcessor -(NVertices-noofVertices));i<noofVerticesinRowofProcessor;i++){
                Aij[i*noofVerticesinRowofProcessor+j] = 0;
            }
        }
     }   
      /* Initializing row and column communicators to facilitate communications for processors in 2-D Distribution */
     MPI_Comm_split(MPI_COMM_WORLD, columnNo, rank, &colComm); 
     MPI_Comm_split(MPI_COMM_WORLD, rowNo, rank, &rowComm);

    for(i=0; i< noofVerticesPerProcessor; i++){
        Fij[i] = F[(rowNo-1)*noofVerticesPerProcessor*noofPRows+(columnNo-1)*noofVerticesPerProcessor +i];
        Pij[i] = Fij[i];
    }
   
    int* rec_buffer = (int*)malloc(sizeof(int)*noofVerticesinRowofProcessor);
    int* send_buffer = (int*)malloc(sizeof(int)*noofVerticesPerProcessor);      

    t2 = MPI_Wtime();  // Marks the end of data set up phase
    if(rank==0)
        printf( "Elapsed time for data reading and initialization %f\n", t2 - t1 ); 

    /* Main Algorithm execution starts here */
    int roundNo=1;
    while(isClear(F,rowNo, columnNo, noofVertices)) {
        if(rank==0){
	     printf("Round No : %d \n", roundNo);
	     roundNo++;
             for(i=0;i<noofVertices;i++){
                 if(F[i]==1)
	             printf("%lld\n",i+1);
             }
	 }
         
         //Updating local view from global frontier
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

        // computing next frontier, and following algorithm details explained in detail in the descrition and report.
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
    }// End of an iteration.
      
    t1 = MPI_Wtime();  // Marks end of the BFS.
    t1 = t1-t2; 
    MPI_Reduce(&t1, &t2,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD); // Computing maximum time elapsed among all processors
    if(rank==0){	
        printf( "Elapsed time for parallel bfs to traverse %lld in %f\n",noofVertices, t2 ); 
        fflush(stdout);
    }
    MPI_Finalize(); // End of execution.
}

long long int getIndex(long long int i, long long int j ,long long int rowSize){
    return i*rowSize+j;
}

bool isClear(int F[],long int rowNo,long int columnNo,long int size) {
    int temp;
    for(temp = 0;temp<size;temp++) {
        if(F[temp]==1) {
            return true;
        }
    }
    return false;
}
