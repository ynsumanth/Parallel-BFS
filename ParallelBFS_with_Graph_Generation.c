#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include "math.h"
#include <stdbool.h>
bool isClear(int[], long int, long int,long int );
long long int getIndex(long long int i,long long int j ,long long int rowSize);

int main (int argc, char *argv[])
{
    unsigned long long int rowNo, columnNo;
    unsigned long long int noofPRows, noofVertices, noofVerticesPerProcessor;
    
    unsigned long long int NVertices;

    int rank, numtasks;
    MPI_Init(&argc, &argv);

    double t1, t2;
    t1 = MPI_Wtime(); 
	
      MPI_Status status;
      MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
      MPI_Comm_rank(MPI_COMM_WORLD,&rank);
      noofPRows = (int) sqrt(numtasks);
      noofVertices = atoll(argv[1]);

      NVertices = ceil(noofVertices*1.0/numtasks)* numtasks;

      
      int dens = atoi(argv[3]);
      noofVerticesPerProcessor = (int) ceil(NVertices/(noofPRows*noofPRows));
//      printf("hello from rank: %d \n",rank);
	
      unsigned long long int i,j,k;
      int* F;
      F = (int *) malloc(sizeof(int)*NVertices);
      for(i=0; i<NVertices; i++){
	F[i] = 0;
      }
      F[atoi(argv[2])] = 1;
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
      
      if(rank==1){
	printf("noofVertices=%d,nVertices=%d, noofVerticesPerProcessor=%d,Ti_size=%d \n ",noofVertices,NVertices, noofVerticesPerProcessor,noofVerticesPerProcessor*noofPRows);
      }
    //Reading File.
      MPI_Comm rowComm, colComm;
      rowNo = (rank)/noofPRows + 1;
      columnNo = rank%noofPRows +1;
      
      int i1,j1; 
      
      i = rank/noofPRows;
      j = rank%noofPRows;
/*
      MPI_Status stat;
      MPI_Datatype MPI_CHUNK;
      MPI_Type_vector(noofVerticesinRowofProcessor,noofVerticesinRowofProcessor,noofVertices,MPI_CHAR,&MPI_CHUNK);
      MPI_Type_commit(&MPI_CHUNK);
      MPI_File fh;
      printf("hello from rank before file open: %d \n",rank);
      MPI_File_open(MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL, &fh);
      printf("i = %d, j=%d hello from rank after file open: %d offset=%llu \n",i,j,rank, (i*noofVertices*noofVerticesinRowofProcessor+j*noofVerticesinRowofProcessor) * sizeof(char));
      MPI_Offset offset = (i*noofVertices*noofVerticesinRowofProcessor+j*noofVerticesinRowofProcessor) * sizeof(char) ;
      printf("offsettt = %llu, i=%d, j=%d \n", offset,i,j);
      MPI_File_set_view(fh, offset, MPI_CHAR, MPI_CHUNK, "native", MPI_INFO_NULL);
       
	printf("before read_all from proc %d with %d , %llu \n",rank, 1ULL*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor, 1ULL*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor);
      if(i != noofPRows-1) 
      MPI_File_read(fh, Aij,1ULL*noofVerticesinRowofProcessor*noofVerticesinRowofProcessor, MPI_CHAR, &stat);
      else 
       MPI_File_read(fh, Aij,1ULL*noofVerticesinRowofProcessor*(noofVerticesinRowofProcessor -(NVertices-noofVertices)), MPI_CHAR, &stat);
      MPI_Type_free(&MPI_CHUNK);
      MPI_File_close(&fh);
  */
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
      printf("noofOnes=%lld\n, density=%f \n",noofOnes,density);
    if(rowNo != columnNo)
        noofOnes *= 2;
    int recv_ddata;  
    MPI_Reduce(&noofOnes, &recv_ddata,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
    if(rank==0){
     printf("sum=%lld, density=%f\n",recv_ddata,recv_ddata*1.0/(noofVertices*noofVertices));
    }

        printf("rank = %d, sum=%d, density=%f\n",rank,recv_ddata,recv_ddata*1.0/(noofVertices*noofVertices));
       char temp;
       long long int chunkSize = 214748364;
//       long long int chunkSize = 10;
       long int  noofChunkstoSend = ceil(((noofVerticesinRowofProcessor*noofVerticesinRowofProcessor)*1.0)/(chunkSize*1.0));
       printf("Data would be sent or received in %d chunks\n",noofChunkstoSend);
       char* Aijtemp = Aij;
       int r;
       if(rowNo < columnNo) {
          for(r=0;r<noofChunkstoSend;r++){
              long long int size = chunkSize;
              if(r==noofChunkstoSend-1){
                  size = noofVerticesinRowofProcessor*noofVerticesinRowofProcessor - r*chunkSize;
              }
              printf("sending %d bytes in round %d \n",size,r+1);
              MPI_Send(Aijtemp, size, MPI_CHAR, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD);
              Aijtemp += size;
          }
       } else if(rowNo > columnNo) {
          for(r=0;r<noofChunkstoSend;r++){
              long long int size = chunkSize;
              if(r==noofChunkstoSend-1){
                  size = noofVerticesinRowofProcessor*noofVerticesinRowofProcessor - r*chunkSize;
              }
              printf("receiving %d bytes in round %d \n",size,r+1);
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


   // printf ("Hello from task %d, row = %d, c=%d \n", rank, rowNo, columnNo);
      for (i=0;i<noofVerticesinRowofProcessor;i++) {
	  for (j=0;j<noofVerticesinRowofProcessor;j++){
           // if(rank==1||rank==2)
	  //  printf("%4d", Aij[i*noofVerticesinRowofProcessor+j]);
	  }
          // if(rank==1||rank==2)
	 // printf("\n");
      } 
      
      MPI_Comm_split(MPI_COMM_WORLD, columnNo, rank, &colComm); 
      MPI_Comm_split(MPI_COMM_WORLD, rowNo, rank, &rowComm);

      
//	printf("Data read in this proc %d ", rank);
	
      for(i=0; i< noofVerticesPerProcessor; i++){
	Fij[i] = F[(rowNo-1)*noofVerticesPerProcessor*noofPRows+(columnNo-1)*noofVerticesPerProcessor +i];
        Pij[i] = Fij[i];
//	printf("Fij[%d] = %4d,  row = %d, c=%d \n",i,Fij[i], rowNo, columnNo);
      }
      
      
      
      int* rec_buffer = (int*)malloc(sizeof(int)*noofVerticesinRowofProcessor);
      int* send_buffer = (int*)malloc(sizeof(int)*noofVerticesPerProcessor);      

      t2 = MPI_Wtime(); 
      if(rank==0)
      	printf( "Elapsed time for data reading and initialization %f\n", t2 - t1 ); 
    //    Pij = F[rank];
        int roundNo=1;
        int noofvertices=0;
        while(isClear(F,rowNo, columnNo, noofVertices)) {
//         for(k=0; k<1; k++){
	  if(rank==0){
	    printf("Round No : %d \n", roundNo);
	    roundNo++;
            for(i=0;i<noofVertices;i++){
               if(F[i]==1)
		  printf("%d\n",i+1);
            }
	  }
          for(i=0; i<noofVerticesPerProcessor; i++){
             send_buffer[i] = Fij[i];
          }
      //  send_buffer[0] = fij;
       // while(isClear(F)) {
         
	  if(rowNo != columnNo) {
	    if(rowNo > columnNo) {
	       MPI_Send(send_buffer, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD);
	       MPI_Recv(Fij, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD, &status);
	     } else {
		MPI_Recv(Fij, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD, &status);
		MPI_Send(send_buffer, noofVerticesPerProcessor, MPI_INT, (columnNo-1)*noofPRows+rowNo-1, 123, MPI_COMM_WORLD); 
	     }
   	  }
//	}
         if(rowNo != columnNo){
	  /*   for(i=0;i<noofVerticesPerProcessor;i++){
       	//	printf("value after transpose r= %d, c=%d Fij[%d] = %d\n", rowNo, columnNo,i,Fij[i]);
             }*/ 
         }
         MPI_Allgather(Fij,noofVerticesPerProcessor,MPI_INT,rec_buffer,noofVerticesPerProcessor,MPI_INT,colComm);
       /*  for(i=0;i<noofVerticesPerProcessor*noofPRows;i++){
        //   printf("value after all gather in  row = %d, c=%d Fi[%d]:%d\n", rowNo, columnNo,i,rec_buffer[i]);
         }*/
  
         int val=0;
         
         for(i=0; i<noofVerticesinRowofProcessor; i++){
             val=0;
             for(j=0;j<noofVerticesinRowofProcessor; j++){
//                if(rank==0)
               // printf("r= %d, c=%d  Aij[%d]*rec_buffer[%d] = %d*%d, actual=%d \n",rowNo, columnNo,i*noofVerticesinRowofProcessor + j,j,Aij[i*noofVerticesinRowofProcessor + j],rec_buffer[j], Aij[i*noofVerticesinRowofProcessor + j]*rec_buffer[j]);
                val += Aij[i*noofVerticesinRowofProcessor + j]*rec_buffer[j];
	     }
             Ti[i] = val;
         }  
         
        /* for(i=0; i<noofVerticesinRowofProcessor; i++){
         //  printf("After mul Ti=%d,from %d, %d \n",Ti[i], rowNo, columnNo);
         }*/
         

         MPI_Alltoall(Ti,noofVerticesPerProcessor,MPI_INT,rec_buffer,noofVerticesPerProcessor,MPI_INT, rowComm);

        /* for(i=0; i<noofVerticesinRowofProcessor; i++){
          //  printf("after all to all recvBuffer=%d,from %d, %d \n",rec_buffer[i], rowNo, columnNo);
         }*/
    //     printf("noofPRows=%d, noofVerticesPerProcessor=%d \n",noofPRows,noofVerticesPerProcessor);
         for(i=0; i<noofPRows; i++){
           for(j=0; j<noofVerticesPerProcessor;j++){
             if(rec_buffer[i*noofVerticesPerProcessor+j] > 0){
                Tij[j]=1;
             }
           }
         }
       /* for(i=0; i<noofVerticesPerProcessor; i++){
        //    printf("after row comm Tij=%d,from %d, %d \n",Tij[i], rowNo, columnNo);
        }*/
        
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
        /* for(i=0; i<noofVerticesinRowofProcessor*noofPRows; i++){
           // if(rank==0)
           // printf("fij=%d,from %d, %d \n",F[i], rowNo, columnNo);
         }*/
       }
      
    t1 = MPI_Wtime(); 
    t1 = t1-t2;
    
    MPI_Reduce(&t1, &t2,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
    if(rank==0){	
       printf( "Elapsed time for parallel bfs to traverse %d in %f\n",noofVertices, t2 ); 
       fflush(stdout);
    }
    MPI_Finalize();
}

long long int getIndex(long long int i, long long int j ,long long int rowSize){
return i*rowSize+j;
}


bool isClear(int F[],long int rowNo,long int columnNo,long int size) {
  int temp,i;
  if(rowNo+columnNo==2)
  for(i=0;i<size;i++){
 //    printf("F[%d]=%d,from %d, %d \n",i,F[i], rowNo, columnNo);
  }
  for(temp = 0;temp<size;temp++) {
    if(F[temp]==1) {
       return true;
    }
  }
  return false;
}
