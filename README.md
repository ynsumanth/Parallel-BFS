# Parallel-BFS
Implementation of Parallel Breadth-First Search on Distributed Memory Systems

Overview:

	In this project, we have implemented the parallel algorithm for Breadth-First Search (BFS), a key subroutine in several graph algorithms. The general structure of the level-synchronous approach holds in case of distributed memory implementations as well, but fine-grained "visited" checks are replaced by edge aggregation-based strategies. With a distributed graph and a distributed d array, a processor cannot tell whether a non-local vertex has been previously visited or not. So the common approach taken is to just accumulate all edges corresponding to non-local vertices, and send them to the owner processor at the end of a local traversal. There is thus an additional all-to-all communication step at the end of each frontier expansion.

	We have implemented a parallel BFS approach with 2D partitioning technique that works well on a dense graph. Let A denote the adjacency matrix of the graph, represented in a dense boolean format, Xk denotes the kth frontier, represented as a vector with integer variables. It is easy to see that the exploration of level k in BFS is algebraically equivalent to Xk+1 = AT x Xk.

	Our dense matrix approach uses the alternative 2D decomposition of the adjacency matrix of the graph. Consider the simple checkerboard partitioning, where processors are logically organized on a square p = pr x pc mesh, indexed by their row and column indices so that the (i; j)th processor is denoted by P(i; j). Edges and vertices (sub-matrices) are assigned to processors according to a 2D block decomposition. Each node stores a sub-matrix of dimensions (n/pr)x(n/pc) in its local memory. We have used two different approaches here:
		1. Generating a dense graph seperately and using MPI FileRead to read only a chunk of data in to the local memory.
		2. Generating a chunk of a dense graph locally in each of the processor by assigning edges randomly.

Implementation Details:

	A brief idea of our algorithm can be depicted as below:
		f(s) <- s
		for all processors P(i, j) in parallel do
			while f != NULL do
			TransposeVector(fij)
			fi <- Allgatherv(fij, P(:,j))
			ti <- Aij X fi
			tij <- Alltoallv(ti, P(i,:))
			tij <- tij X Pij
			Pij <- Pij + tij
			fij <- tij

	This algorithm computes the "breadth-first spanning tree" by returning a dense parents array. "s" denotes the source vertex which is an input to the algorithm. "f" denotes the complete frontier vector while fij denotes the frontier vector of each individual processor P(i,j). "fi" denotes the frontier vector of an entire column of processors 'pc'. "tij" and "ti" are intermediate vectors owned by the individual and an entire column of processors. TransposeVector redistributes the vector so that the subvector owned by the ith processor row is now owned by the ith processor column. In the case of pr = pc = sqrt(p), the operation is simply a pair-wise exchange between P(i,j) and P(j,i). Allgather(fij , P(:, j)) syntax denotes that subvectors fij for j = 1,..,pr is accumulated at all the processors on the jth processor column. Similarly, Alltoallv(ti, P(i, :)) denotes that each processor scatters the corresponding piece of its intermediate vector "ti" to its owner, along the ith processor row.

	

Experimental Studies and Analysis:

	Analysis 1:
	The table below represents the time taken for traversing the specified number of vertices. For analysis purposes, we have fixed the number of cores that our algorithm will be run on and the input graph considered is 60% dense. With increasing number of vertices, which is the problem size, our algorithm scales as desired.
	
	NoofCores	Memory Per Core	No of Vertices	Density	Time for Init	Time for traversing	No of edges
	64			32768			16384			60											
	64			32768			32768			60			
	64			32768			65536			60			
	64			32768			131072			60			
	64			32768			262144			60			
	64			32768			524288			60			
	64			32768			1048576			60			
	64			65536			1900000			60			
	
	Analysis 2:
	Here, we have fixed the problem size i.e. the input graph is of 1M vertices and 60% dense. As the number of cores increases, the time taken to traverse follows the desired pattern.
	
	NoofCores	Memory Per Core	No of Vertices	Density	Time for Init	Time for traversing	No of edges
	25			65536			1048576			60			
	36			32768			1048576			60			
	49			32768			1048576			60			
	64			32768			1048576			60			
	81			16384			1048576			60		530 sec	
	
	Analysis 3:
	The table here explains the behavior of algorithm with fixed problem size, and fixed number of cores used but with varying complexity of the problem.
	
	NoofCores	Memory Per Core	No of Vertices	Density	Time for Init	Time for traversing	No of edges
	64			32768			1048576			30			
	64			32768			1048576			40			
	64			32768			1048576			50			
	64			32768			1048576			60			
	64			32768			1048576			70			
	64			32768			1048576			80			
	64			32768			1048576			90			
