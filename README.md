Overview:

In this project, we have implemented a distributed parallel algorithm for Breadth-First Search (BFS), a key subroutine in several graph algorithms. The general structure of the level-synchronous approach holds in case of distributed memory implementations as well, but fine-grained "visited" checks are replaced by edge aggregation-based strategies. With a distributed graph and a distributed d array, a processor cannot tell whether a non-local vertex has been previously visited or not. So the common approach taken is to just accumulate all edges corresponding to non-local vertices, and send them to the owner processor at the end of a local traversal. There is thus an additional all-to-all communication step at the end of each frontier expansion.
We have implemented a parallel BFS approach with 2D partitioning technique that works well on a dense graph. Let A denote the adjacency matrix of the graph, represented in a dense boolean format, Xk denotes the kth frontier, represented as a vector with integer variables. It is easy to see that the exploration of level k in BFS is algebraically equivalent to Xk+1 = AT x Xk.
Our dense matrix approach uses the alternative 2D decomposition of the adjacency matrix of the graph. Consider the simple checkerboard partitioning, where processors are logically organized on a square p = pr x pc mesh, indexed by their row and column indices so that the (i,j)th processor is denoted by P(i, j). Edges and vertices (sub-matrices) are assigned to processors according to 2D block decomposition. Each node stores a sub-matrix of dimensions (n/pr)x(n/pc) in its local memory. 

Assumptions:

We implemented this parallel BFS which is taken from the paper [1] by Kamesh Madduri et al. The following are few assumption and modifications we made:
1)	Input graph for parallel traversing is assumed to be dense, and hence we use adjacency matrix representation for input graph.
2)	Number of processor used to solve this problem in parallel is always a perfect square. This constraint is attributed to the fact that there is transpose communication between processor as explained in implementation details below.
3)	We have worked only on undirected graphs, we are confident of extending the same implementation to directed graphs too.
We have ignored input data generation time while analyzing our algorithm.

Data set generation and Processing:

We have implemented two different versions of our algorithm to generate input graph to our parallel BFS algorithm. They are briefly listed below:
1.	We have generated the input graph separately using the Erdos-Renyi model. This model takes the input seed, probability and the source vertex to generate the adjacency matrix of the graph. In this data generation approach, our BFS algorithm uses the MPI File Read to read data into local memory in parallel.
2.	In the second version, we have generated the adjacency matrix for each processor locally. All the processors generate a part of the graph by randomly assigning 1’s and 0’s depending on the density parameter. We also made sure that the undirected input graph/ adjacency matrix as a whole is symmetric by communications between processors above the diagonal to the processors below the diagonal in the grid. Hence, the right& above half of the adjacency matrix is perfect mirror to left & below half of the matrix.
Also, if the input adjacency matrix is not sufficient to get distributed across all the processors equally, we appended 0’s in the last few rows/ last few columns of the matrix. This ensures that each processor gets equal number of vertices in the partitioning and the length for the distributed frontier vector in our algorithm remains same for each processor.

Implementation Details:

	A brief idea of our algorithm can be depicted as below:
		f(s)  s
		For all processors P(i, j) in parallel do
			While F != NULL do
			TransposeVector(fij)
			fi Allgatherv(fij, P(:,j))
			ti  Aij X fi
			tij  Alltoallv(ti, P(i,:))
			tij  tij X Pij
			Pij  Pij + tij
			fij  tij
This algorithm computes the "breadth-first spanning tree" by returning a dense parents array. "s" denotes the source vertex which is an input to the algorithm. "f" denotes the complete frontier vector while fij denotes the frontier vector of each individual processor P(i,j). "fi" denotes the frontier vector of an entire column of processors 'pc'. "tij" and "ti" are intermediate vectors owned by the individual and an entire column of processors. TransposeVector redistributes the vector so that the subvector owned by the ith processor row is now owned by the ith processor column. In the case of pr = pc = √p, the operation is simply a pair-wise exchange between P(i,j) and P(j,i). Allgather(fij , P(:, j)) syntax denotes that subvectors fij for j = 1,..,pr is accumulated at all the processors on the jth processor column. Similarly, Alltoallv(ti, P(i, :)) denotes that each processor scatters the corresponding piece of its intermediate vector "ti" to its owner, along the ith processor row.

Experimental Studies and Analysis:

Analysis 1:

NoofCores	No of Vertices	density	Time for Traversing	speedup		Efficiency
1		65536		60	50.397845		0	
9		65536		60	8.213346		6.136092	0.681788
16		65536		60	6.164028		8.176122	0.511008
25		65536		60	4.265182		11.81611	0.472644
36		65536		60	3.258334		15.46737	0.429649
49		65536		60	2.691904		20.30262	0.414339

1		131072		60	202.290836		0	
9		131072		60	37.225393		5.434216	0.603802
16		131072		60	22.902601		8.832658	0.552041
25		131072		60	15.296578		13.22458	0.528983
36		131072		60	11.172929		15.56825	0.432451
49		131072		60	8.731373		20.77074	0.423893

1		262144		60	795.938821		0	
9		262144		60	155.344089		5.123715	0.569302
16		262144		60	83.845994		9.492866	0.593304
25		262144		60	58.189566		13.67838	0.547135
36		262144		60	41.251957		19.29457	0.53596
49		262144		60	31.300927		25.4113	0.518598
 
The above graph shows the Strong scalability results with Speedup plotted against the number of cores. Here problem size remains as constant (for each individual curve) with increasing processors. As the processors increases, our speedup curve falls short of the expected linear speedup due to the increased communication overhead. Additionally, it can be inferred that the speedup curves of our algorithm grows in proportion to 2√P, where P is the number of processors.
The goal of the strong scalability plot is to find the “Sweet-spot” that allows us to find the number of processors for efficient computations, yet not wasting too many cycles due to parallel overhead. From the above graph, we conclude that the “sweet-spot” of our algorithm is using 25 processors as the speedup curves are scaling as expected. But if the number of processors exceeds 25, the efficiency of the algorithm drops due to the parallel overhead and hence the speedup doesn’t grow as expected.
 
•	From the above efficiency graph, two conclusions can be drawn. With a fixed problem size, as the number of cores increases, efficiency of the algorithm decreases. We attribute this decrease in efficiency to the communication overhead with increasing number of processors.
•	Also we observe from above data that, for the same number of cores, as the problem size increases, efficiency increases.	

Analysis 2:

Number of Cores	Number of Vertices	Density	Time for Traversal
64		1048576			20	420.658304
64		1048576			30	574.846067
64		1048576			40	615.235705
64		1048576			50	619.524863
64		1048576			60	620.12821
64		1048576			70	616.00771
64		1048576			80	624.226022
64		1048576			90	614.899241

 
The main observation from the above graph is the following:
•	With a density of 40% and beyond, traversal time doesn’t change significantly. We attribute this saturation to the fact that there are minimal changes to the frontier vector as the density grows. The frontier vector is instrumental in deciding the number of iterations the algorithm has to perform.
•	For 20% and 30% dense graphs, we infer that the graph itself is sparsely connected and due to disconnections in the input graph, the time taken is significantly low compared to others.

Analysis 3:

Number of Cores	   Number of Vertices	Density	   Time for Traversal	Edges visited Per Sec
25			1048576		60	1525.501779	687.3646524
36			1048576		60	1156.483432	906.6934908
49			1048576		60	810.603978	1293.573716
64			1048576		60	614.640618	1705.998545
81			1048576		60	530.922523	1975.007566

 
The graph plotted above depicts the number of edges traversed per second on a 1Million input graph of 60% density. With increasing number of processors, the number of edges visited per second increases in a linear fashion. We attribute this to the fact that as processors increases, the input graph breaks down among these cores and each core takes responsibility of traversing their local graph. Thus number of edges visited equals to the edges traversed by each processor in the fixed interval of time, thereby increasing the value of Edge traversal with processors.

Analysis 4:

Number of Cores	     Time for Traversal	Number of Vertices
64			0.220598	16384
64			0.703791	32768
64			3.24984	65536
64			10.416746	131072
64			38.503266	262144
64			153.579155	524288
64			620.12821	1048576


 
The graph above shows the curve for Time taken for graph traversal with varied problem size. Here the number of processors is fixed to 64 cores. As we can see with the increasing size of the input graph, the time taken for the traversal of the graph increases exponentially. Thus it can be observed that, to solve the bigger problems in less time, we require more number of processors.

References:

[1] http://gauss.cs.ucsb.edu/~aydin/sc11_bfs.pdf.
[2] http://www.mpich.org/static/docs/v3.1/www3/
[3] http://gauss.cs.ucsb.edu/~aydin/sc11_bfs.pdf
[4] http://www.cc.gatech.edu/fac/bader/papers/TaskBFS-HiPC2012.pdf
[5] http://www.boost.org/doc/libs/1_58_0/libs/graph/doc/erdos_renyi_generator.html
[6] http://mpi.deino.net/mpi_functions/MPI_Comm_split.html
