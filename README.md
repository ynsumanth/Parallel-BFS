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

How to run:

Compile the source code using MPICC command.
ex: mpicc PBFSWithGraphGeneration.c -o base-bfs -lm

Now run the output file base-bfs using MPIRUN command.
ex: mpirun -np 16 base-bfs 131072 1

Also, a SLURM job can be submitted using the SRUN command.
srun --n_procs base-bfs 131072 1


References:

[1] http://gauss.cs.ucsb.edu/~aydin/sc11_bfs.pdf.
[2] http://www.mpich.org/static/docs/v3.1/www3/
[3] http://gauss.cs.ucsb.edu/~aydin/sc11_bfs.pdf
[4] http://www.cc.gatech.edu/fac/bader/papers/TaskBFS-HiPC2012.pdf
[5] http://www.boost.org/doc/libs/1_58_0/libs/graph/doc/erdos_renyi_generator.html
[6] http://mpi.deino.net/mpi_functions/MPI_Comm_split.html
