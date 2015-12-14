#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#define MAX 262144
 
typedef enum boolean{false, true} bool;
char** adj;
bool visited[ MAX ];
main()
{
    int i, v=1;
    create_graph();
    for ( i = 0;i < MAX;i++ )
	visited[ i ] = false;
   clock_t start = clock(), diff;
bfs(v);
diff = clock() - start;

int msec = diff * 1000 / CLOCKS_PER_SEC;
printf("Time taken %d seconds %d milliseconds", msec/1000, msec%1000);
 } /*End of main()*/
 
create_graph()
{
  //printf("1\n");
  adj = (char **) malloc(sizeof(char*)*MAX);

  int ind=0; 
  for(;ind<MAX;ind++)
     adj[ind] = (char *) malloc(MAX*sizeof(char));
   srand(100);
   int dens = 60;
      long long int noofOnes=0;
      long long int i2,j2,k2;
      printf("2\n");
         for(i2=0; i2< MAX; i2++){
	   for(j2=i2; j2< MAX; j2++){
//              printf("%d e ",i2); 	     
              int rand1 = rand()%100;
              if(rand1>=dens){
                adj[i2][j2] = 1;
		adj[j2][i2] = 1;
                noofOnes++;
              }else{
		adj[i2][j2] = 0;
		adj[j2][i2] = 0;
	      }
	   }
      }
      printf("3\n");
     // display();

} /*End of create_graph()*/
 
display()
{
    int i, j;
   printf("inside display\n");
    for ( i = 0;i < MAX;i++ )
        {
            for ( j = 0;j < MAX;j++ )
                printf( "%4d", adj[ i ][ j ] );
 
            printf( "\n" );
        }
} /*End of display()*/

 
bfs( int v )
{
    long long int i, front, rear;
    int* que = (int*) malloc(sizeof(int)*MAX);
    front = rear = -1;
 
    printf( "%d ", v );
    visited[ v ] = true;
    rear++;
    front++;
    que[ rear ] = v;
 
    while ( front <= rear )
        {
            v = que[ front ]; /* delete from queue */
            front++;
 
            for ( i = 0;i < MAX;i++ )
                {
                    /* Check for adjacent unvisited nodes */
 
                    if ( adj[ v ][ i ] == 1 && visited[ i ] == false )
                        {
                            printf( "%d ", i );
                            visited[ i ] = true;
                            rear++;
                            que[ rear ] = i;
                        }
                }
	}
} /*End of bfs()*/
 
adj_nodes( int v )
{
    int i;
 
    for ( i = 0;i < MAX;i++ )
        if ( adj[ v ][ i ] == 1 )
            printf( "%d ", i );
 
    printf( "\n" );
} /*End of adj_nodes()*/
 
