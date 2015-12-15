#include <iostream>
#include <random>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>


int main(int argc, char* argv[]) {
    typedef boost::adjacency_list<boost::vecS,boost::vecS,boost::undirectedS> Graph;
    typedef boost::erdos_renyi_iterator<std::mt19937, Graph> erg;

    if (argc < 3) return -1;

    long int n = std::atoi(argv[1]);
    double eps = std::atof(argv[2]);

    std::random_device rd;
    int seed = -1;

    long int s = 0 ;
    if (argc > 3) seed = std::atoi(argv[3]);
    if (argc == 5) s = std::atol(argv[4]);

    if (seed == -1) seed = rd();
    double p = (eps / n);
    std::ofstream myfile;
    myfile.open(argv[5],std::ios::binary);

    std::mt19937 rng(seed);
    Graph G(erg(rng, n, p), erg(), n);

    boost::graph_traits<Graph>::vertex_iterator e, end;
    std::tie(e, end) = boost::vertices(G);

    boost::graph_traits<Graph>::in_edge_iterator inEdge, inEdgeEnd;

    boost::graph_traits<Graph>::edge_iterator temp, temp_end;
    std::tie(temp, temp_end) = boost::edges(G);
    int *buf= new int[n];
    int k= 0, pos=0;
    std::vector<char> buffer(32 * 1024 * 1024*50);
    char one[1], zero[1];
    one[0] = '\x1';
    zero[0] = '\x0';
    long long int edgeCount =0;
    for (; e != end; ++e) {
	std::fill_n(buf,n,0);
	std::tie(inEdge, inEdgeEnd) = boost::in_edges(*e, G);                     
	for(;inEdge != inEdgeEnd; ++inEdge) {
    	   int index_of_source = boost::source(*inEdge, G);
	   buf[index_of_source] = 1;
	}
 	for(k=0;k<n;k++) {                      
            if(buf[k]==0){
                memcpy ( &buffer[pos], zero, 1 );
            }
            else{
                memcpy ( &buffer[pos], one, 1 );
                edgeCount++;
            }
            pos += 1;                             
            if (pos >= buffer.size()-8) {
       	        myfile.write(buffer.data(), pos * sizeof(char));
                pos = 0;
            } 
        }
    }
    myfile.write(reinterpret_cast<char*>(buffer.data()), pos * sizeof(char));
    myfile.close();
    std::cout<<edgeCount<<std::endl;
    return 0;
}
