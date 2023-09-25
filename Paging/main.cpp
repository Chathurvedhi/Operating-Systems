#include<bits/stdc++.h>
using namespace std;

class MEM
{
    private:
    int mainsize, virtsize, pagesize;
    string infile, outfile;
    unordered_map<int, unordered_map<int, int>> proc_mem_map;

    public:

    // Constructor
    MEM(int M, int V, int P, string infile, string outfile)
    {
        // Initialize the variables
        mainsize = M;
        virtsize = V;
        pagesize = P;
        this->infile = infile;
        this->outfile = outfile;
    }  

    // Function to run the program
    void run()
    {
    
    } 

};

int main(int argc, char* argv[])
{
    // Take input for Main Mem Size(M), Virt Mem Size(V), Page Size(P), infile outfile using arguments
    int M, V, P;
    string infile, outfile;
    for(int i=1; i<argc; i++)
    {
        if(strcmp(argv[i], "-M")==0)
        {
            M = stoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-V")==0)
        {
            V = stoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-P")==0)
        {
            P = stoi(argv[i+1]);
        }
        else if(strcmp(argv[i], "-i")==0)
        {
            infile = argv[i+1];
        }
        else if(strcmp(argv[i], "-o")==0)
        {
            outfile = argv[i+1];
        }
    }

    MEM mem = MEM(M, V, P, infile, outfile);
    mem.run();
}   