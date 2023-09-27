#include<bits/stdc++.h>
using namespace std;

// Global variables
bool verbose = false;
int global_pagesize;

class proc
{
    public:
    string filename;
    int pid;
    int size;
    int location;
    unordered_map<int, int> page_table;     // Maps page number to frame number
};

class MEM
{
    private:
    int mainsize, virtsize, pagesize;
    int mainpages, virtpages;
    int mainoccupied, virtoccupied;
    int pid_counter;
    unordered_map<int, proc> pid_to_file;
    unordered_set<int> virtpid;    
    queue<int> mainpid;                     // Store pids in order of LRU
    queue<int> main_free;                   // Store free pages in main memory
    queue<int> virt_free;                   // Store free pages in virtual memory
    unordered_map<int, int> main_mem;       // Memory locations in main memory storing an integer
    unordered_map<int, int> virt_mem;       // Memory locations in virtual memory storing an integer

    string infile, outfile;

    public:

    // Constructor
    MEM(int M, int V, int P, string infile, string outfile)
    {
        // Initialize the variables
        mainsize = M;
        virtsize = V;
        pagesize = P;
        global_pagesize = P;
        this->infile = infile;
        this->outfile = outfile;
        mainpages = mainsize*1024/pagesize;
        virtpages = virtsize*1024/pagesize;
        for(int i=0; i<mainpages; i++) main_free.push(i);
        for(int i=0; i<virtpages; i++) virt_free.push(i);
        mainoccupied = 0;
        virtoccupied = 0;
        pid_counter = 1;
    }  

    // Function to load the process
    void load(string line)
    {
        // Go through all files and process them
        while(line.length() > 0)
        {
            string filename = line.substr(0, line.find(" "));
            line = line.substr(line.find(" ")+1);
            if(filename == line) line = "";
            if(verbose) cout << filename << endl;

            ofstream fout(outfile, ios::app);

            // Check if file exists
            ifstream fin(filename);
            if(!fin)
            {
                if(verbose) cout << "No file" << endl;
                fout<< filename << " could not be loaded - file does not exist"<<endl;
                continue;
            }

            // Read first line of file and convert to int
            string firstline;
            getline(fin, firstline);
            int size = stoi(firstline);

            // Create proc object
            proc p;
            p.filename = filename;
            p.pid = pid_counter;
            pid_counter++;
            p.size = size;
            p.location = -1;

            // Place proc in main memory or virtual memory based on size

            int mem_allocated = -1;

            if(size <= mainsize - mainoccupied)
            {
                // Place in main memory
                p.location = 0;
                mem_allocated = 0;
                mainoccupied += size;
                mainpid.push(p.pid);

                // Create page table
                int pages = size*1024/pagesize;
                for(int i=0; i<pages; i++)
                {
                    p.page_table[i] = main_free.front();
                    main_free.pop();
                }
            }
            else if(size <= virtsize - virtoccupied)
            {
                // Place in virtual memory
                p.location = 1;
                mem_allocated = 1;
                virtoccupied += size;
                virtpid.insert(p.pid);

                // Create page table
                int pages = size*1024/pagesize;
                for(int i=0; i<pages; i++)
                {
                    p.page_table[i] = virt_free.front();
                    virt_free.pop();
                }
            }
            else
            {
                if(verbose) cout << "Cannot place in memory" << endl;
                // Cannot place in memory
                fout<< filename << " could not be loaded - memory is full"<<endl;
                continue;
            }

            // Map file with pid
            pid_to_file[p.pid] = p;

            // Print to outfile
            if(mem_allocated == 0)
            {
                if(verbose) cout << filename << " in main memory" << endl;
                fout<< filename << " is loaded in main memory and is assigned process id "<<p.pid<<endl;
            }
            else if(mem_allocated == 1)
            {
                if(verbose) cout << filename << " in virtual memory" << endl;
                fout<< filename << " is loaded in virtual memory and is assigned process id "<<p.pid<<endl;
            }
        }
    }

    // Function to kill a process
    void kill(int pid)
    {
        ofstream fout(outfile, ios::app);

        // Check if pid exists in pid_to_file unordered map
        if(pid_to_file.find(pid) == pid_to_file.end())
        {
            if(verbose) cout << "No process with pid " << pid << endl;
            fout<< "Process with pid "<<pid<<" does not exist"<<endl;
            return;
        }
        
        // Check if pid is in main memory or virtual memory
        if(pid_to_file[pid].location == 0)
        {
            // Remove from main memory
            mainoccupied -= pid_to_file[pid].size;
            queue<int> temp;
            while(mainpid.front() != pid)
            {
                temp.push(mainpid.front());
                mainpid.pop();
            }
            mainpid.pop();
            while(!temp.empty())
            {
                mainpid.push(temp.front());
                temp.pop();
            }

            // Remove from pid_to_file
            pid_to_file.erase(pid);

            // Add pages to free list
            for(auto it = pid_to_file[pid].page_table.begin(); it != pid_to_file[pid].page_table.end(); it++)
            {
                main_free.push(it->second);
            }

            // Output to outfile
            fout<< "Process with pid "<<pid<<" is killed from main memory"<<endl;
        }
        else if(pid_to_file[pid].location == 1)
        {
            // Remove from virtual memory
            virtoccupied -= pid_to_file[pid].size;
            virtpid.erase(pid);

            // Remove from pid_to_file
            pid_to_file.erase(pid);

            // Add pages to free list
            for(auto it = pid_to_file[pid].page_table.begin(); it != pid_to_file[pid].page_table.end(); it++)
            {
                virt_free.push(it->second);
            }

            // Output to outfile
            fout<< "Process with pid "<<pid<<" is killed from virtual memory"<<endl;
        }

    }
    
    // Function to list the processes
    void listpr()
    {
        ofstream fout(outfile, ios::app);

        // Print all processes in main memory in order of pid
        fout<< "Processes in main memory: ";
        priority_queue<int, vector<int>, greater<int>> temp;
        queue<int> main_temp;
        main_temp = mainpid;
        while(!main_temp.empty())
        {
            temp.push(main_temp.front());
            main_temp.pop();
        }
        while(!temp.empty())
        {
            fout<< temp.top() << " ";
            temp.pop();
        }

        // Print all processes in virtual memory in order of pid
        fout<<endl<<"Processes in virtual memory: ";
        unordered_set<int> virt_temp;
        virt_temp = virtpid;
        for(auto it = virt_temp.begin(); it != virt_temp.end(); it++)
        {
            temp.push(*it);
        }
        while(!temp.empty())
        {
            fout<< temp.top() << " ";
            temp.pop();
        }
        fout<<endl;
    }
    
    // Function to print the page table entry of a process
    void pte(string line, int flag = 0)
    {
        // Split into pid and filename
        int pid = stoi(line.substr(0, line.find(" ")));
        line = line.substr(line.find(" ")+1);
        string pteoutfile = line.substr(0, line.find(" "));

        ofstream fout(pteoutfile, ios::app);

        // Print time and date of execution in milliseconds
        time_t now = time(0);
        char* dt = ctime(&now);
        if(flag == 1) fout<<dt<<endl;

        // Check if pid exists in pid_to_file unordered map
        if(pid_to_file.find(pid) == pid_to_file.end())
        {
            if(verbose) cout << "No process with pid " << pid << endl;
            fout<< "Process with pid "<<pid<<" does not exist"<<endl;
            return;
        }

        // Check if pid is in main memory or virtual memory
        if(pid_to_file[pid].location == 0)
        {
            // Print page table entry to outfile
            fout<< "Page Table entry for process with pid "<<pid<<" is:"<<endl;
            fout << "Logical Page Number and Physical Page Number in Main Memory" << endl;
            
            // Print in ascending order of logical page number
            priority_queue<int, vector<int>, greater<int>> temp;
            for(auto it = pid_to_file[pid].page_table.begin(); it != pid_to_file[pid].page_table.end(); it++)
            {
                temp.push(it->first);
            }

            while(!temp.empty())
            {
                fout << temp.top() << " " << pid_to_file[pid].page_table[temp.top()] << endl;
                temp.pop();
            }
        }
        else if(pid_to_file[pid].location == 1)
        {
            // Print page table entry to outfile
            fout<< "Page Table entry for process with pid "<<pid<<" is:"<<endl;
            fout << "Logical Page Number and Physical Page Number in Virtual Memory" << endl;

            // Print in ascending order of logical page number
            priority_queue<int, vector<int>, greater<int>> temp;
            for(auto it = pid_to_file[pid].page_table.begin(); it != pid_to_file[pid].page_table.end(); it++)
            {
                temp.push(it->first);
            }

            while(!temp.empty())
            {
                fout << temp.top() << " " << pid_to_file[pid].page_table[temp.top()] << endl;
                temp.pop();
            }
        }

        if(flag == 1) fout << "----------------------------------------" << endl;

        ofstream fout_main(outfile, ios::app);
        if(flag == 1) fout_main << "Printed page table entry of process with pid " << pid << " in " << pteoutfile << endl;
    }

    // Function to print the page table entry of all processes
    void pteall(string line)
    {
        ofstream fout(line, ios::app);

        if(verbose) cout << "Func called: pteall in call" << endl; 

        // Print time and date of execution
        time_t now = time(0);
        char* dt = ctime(&now);
        fout<<dt<<endl;

        // Print page table entry of all processes in ascending order of pid
        priority_queue<int, vector<int>, greater<int>> temp;
        for(auto it = pid_to_file.begin(); it != pid_to_file.end(); it++)
        {
            temp.push(it->first);
        }

        while(!temp.empty())
        {
            pte(to_string(temp.top()) + " " + line, 0);
            temp.pop();
        }

        fout << "----------------------------------------" << endl;

        ofstream fout_main(outfile, ios::app);
        fout_main << "Printed page table entries of all processes in " << line << endl;
    }

    // Function to swap out a process
    void swapout(int pid)
    {
        ofstream fout(outfile, ios::app);

        // Check if pid exists in mainpid
        queue<int> temp;
        bool found = false;
        while(!mainpid.empty())
        {
            if(mainpid.front() == pid)
            {
                found = true;
                mainpid.pop();
                break;
            }
            temp.push(mainpid.front());
            mainpid.pop();
        }
        while(!temp.empty())
        {
            mainpid.push(temp.front());
            temp.pop();
        }
        if(!found)
        {
            if(verbose) cout << "No process with pid " << pid << endl;
            fout<< "Process with pid "<<pid<<" does not exist in Main Memory"<<endl;
            return;
        }

        // Deallocate pages in main memory and move to virtual memory
        mainoccupied -= pid_to_file[pid].size;
        pid_to_file[pid].location = 1;
        virtpid.insert(pid);
        virtoccupied += pid_to_file[pid].size;
        int pages = pid_to_file[pid].size*1024/pagesize;
        for(int i=0; i<pages; i++)
        {
            // Swap out page locations
            int main_mem_page_num = pid_to_file[pid].page_table[i];
            main_free.push(main_mem_page_num);
            pid_to_file[pid].page_table[i] = virt_free.front();
            int virt_mem_page_num = virt_free.front();
            virt_free.pop();

            // Move memory from main_mem to virt_mem
            int main_mem_loc = main_mem_page_num*pagesize;
            int virt_mem_loc = virt_mem_page_num*pagesize;
            for(int j=0; j<pagesize; j++)
            {
                virt_mem[virt_mem_loc + j] = main_mem[main_mem_loc + j];
            }
        }

        // Output to outfile
        fout<< "Process with pid "<<pid<<" is swapped out from main memory"<<endl;
    }

    // Function to swap in a process
    void swapin(int pid)
    {
        ofstream fout(outfile, ios::app);

        // Check if pid exists in virtpid
        if(virtpid.find(pid) == virtpid.end())
        {
            if(verbose) cout << "No process with pid " << pid << endl;
            fout<< "Process with pid "<<pid<<" does not exist in Virtual Memory"<<endl;
            return;
        }

        unordered_map<int, int> virt_mem_temp = virt_mem;
        unordered_map<int, int> proc_page_table_temp = pid_to_file[pid].page_table;
        
        // Deallocate memory in virtual memory in order of virtual page numbers
        virtpid.erase(pid);
        virtoccupied -= pid_to_file[pid].size;
        int pages = pid_to_file[pid].size*1024/pagesize;
        for(int i=0; i<pages; i++) virt_free.push(pid_to_file[pid].page_table[i]);

        while(pid_to_file[pid].size > mainsize - mainoccupied)
        {
            // Swap out the LRU process
            if (verbose) cout << "Swapping out process with pid " << mainpid.front() << endl;
            swapout(mainpid.front());
        }

        // Now allocate memory in main memory
        pid_to_file[pid].location = 0;
        mainoccupied += pid_to_file[pid].size;
        mainpid.push(pid);
        for(int i=0; i<pages; i++)
        {
            // Swap in page locations
            int virt_mem_page_num = proc_page_table_temp[i];
            pid_to_file[pid].page_table[i] = main_free.front();
            int main_mem_page_num = main_free.front();
            main_free.pop();

            // Move memory from virt_mem to main_mem
            int virt_mem_loc = virt_mem_page_num*pagesize;
            int main_mem_loc = main_mem_page_num*pagesize;
            for(int j=0; j<pagesize; j++)
            {
                main_mem[main_mem_loc + j] = virt_mem[virt_mem_loc + j];
            }
        }

        // Output to outfile
        fout<< "Process with pid "<<pid<<" is swapped into main memory"<<endl;
        return;
    }

    // Function to print the memory in main memory
    void mem_print(string line)
    {
        ofstream fout(outfile, ios::app);
        
        // Split line into memloc and length

        int memloc = stoi(line.substr(0, line.find(" ")));
        line = line.substr(line.find(" ")+1);
        int length = stoi(line);
        if(verbose) cout << "Memloc: " << memloc << ", Length: " << length << endl;

        // Print memory allocated in main memory
        while(length--)
        {
            fout << "Value at "  << memloc << ": " << main_mem[memloc] << endl;
            memloc++;
        }
    }

    void run_command(int pid)
    {
        ofstream fout(outfile, ios::app);

        // Check if pid exists in pid_to_file unordered map
        if(pid_to_file.find(pid) == pid_to_file.end())
        {
            if(verbose) cout << "No process with pid " << pid << endl;
            fout<< "Process with pid "<<pid<<" does not exist"<<endl;
            return;
        }

        // If process is in virtual memory, swap it in
        if(pid_to_file[pid].location == 1)
        {
            swapin(pid);
        }

        // Input custom commands from file with pid
        string filename = pid_to_file[pid].filename;
        ifstream fin(filename);
        string line;
        bool temp = true;

        while(getline(fin, line))
        {
            if(temp)
            {
                temp = false;
                continue;
            }

            string first = line.substr(0, line.find(" "));
            line = line.substr(line.find(" ")+1);

            // Compare with load, add, sub, print
            if(first.compare("load")==0)
            {
                fout << "Command: " << "load " << line << endl;
                // Line: num, virtmemloc
                int num = stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ")+1);
                int virtmemloc = stoi(line);

                // Check if virtmemloc is valid
                if(virtmemloc > pid_to_file[pid].size*1024)
                {
                    if(verbose) cout << "Invalid virtual memory location" << endl;
                    fout << "Invalid virtual memory location" << endl;
                    continue;
                }

                // Convert virtmemloc to page number and offset
                int page_num = virtmemloc/pagesize;
                int offset = virtmemloc%pagesize;
                
                // Convert page number to main memory location
                int main_mem_loc = pid_to_file[pid].page_table[page_num]*pagesize + offset;
                if(verbose) cout << "Main mem loc: " << main_mem_loc << endl;

                // Store num in main memory
                main_mem[main_mem_loc] = num;
                fout << "Result: Value of addr " << virtmemloc << " is changed to " << num << endl;
            }
            else if(first.compare("add")==0)
            {
                fout << "Command: " << "add " << line << endl;
                // Line: x, y, z (z = x + y)(x, y, z are virtual memory locations)
                int x = stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ")+1);
                int y = stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ")+1);
                int z = stoi(line);

                // Check if x, y, z are valid
                if(x > pid_to_file[pid].size*1024 || y > pid_to_file[pid].size*1024 || z > pid_to_file[pid].size*1024)
                {
                    if(verbose) cout << "Invalid virtual memory location" << endl;
                    fout << "Invalid virtual memory location" << endl;
                    continue;
                }

                // Convert x, y, z to page number and offset
                int x_page_num = x/pagesize;
                int x_offset = x%pagesize;
                int y_page_num = y/pagesize;
                int y_offset = y%pagesize;
                int z_page_num = z/pagesize;
                int z_offset = z%pagesize;

                // Convert page number to main memory location
                int x_main_mem_loc = pid_to_file[pid].page_table[x_page_num]*pagesize + x_offset;
                int y_main_mem_loc = pid_to_file[pid].page_table[y_page_num]*pagesize + y_offset;
                int z_main_mem_loc = pid_to_file[pid].page_table[z_page_num]*pagesize + z_offset;

                // Add the numbers, if not found, they are 0
                int sum = main_mem[x_main_mem_loc] + main_mem[y_main_mem_loc];
                if(verbose) cout << "Sum: " << sum << endl;
                main_mem[z_main_mem_loc] = sum;
                if(verbose) cout << "Value of addr " << x << " = " << main_mem[x_main_mem_loc] << ", Value of addr " << y << " = " << main_mem[y_main_mem_loc] << ", Value of addr " << z << " = " << main_mem[z_main_mem_loc] << endl;
                
                fout << "Result: Value of addr " << x << " = " << main_mem[x_main_mem_loc] << ", Value of addr " << y << " = " << main_mem[y_main_mem_loc] << ", Value of addr " << z << " = " << main_mem[z_main_mem_loc] << endl;
            }
            else if(first.compare("sub")==0)
            {
                fout << "Command: " << "sub " << line << endl;
                // Line: x, y, z (z = x - y)(x, y, z are virtual memory locations)
                int x = stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ")+1);
                int y = stoi(line.substr(0, line.find(", ")));
                line = line.substr(line.find(", ")+1);
                int z = stoi(line);

                // Check if x, y, z are valid
                if(x > pid_to_file[pid].size*1024 || y > pid_to_file[pid].size*1024 || z > pid_to_file[pid].size*1024)
                {
                    if(verbose) cout << "Invalid virtual memory location" << endl;
                    fout << "Invalid virtual memory location" << endl;
                    continue;
                }

                // Convert x, y, z to page number and offset
                int x_page_num = x/pagesize;
                int x_offset = x%pagesize;
                int y_page_num = y/pagesize;
                int y_offset = y%pagesize;
                int z_page_num = z/pagesize;
                int z_offset = z%pagesize;

                // Convert page number to main memory location
                int x_main_mem_loc = pid_to_file[pid].page_table[x_page_num]*pagesize + x_offset;
                int y_main_mem_loc = pid_to_file[pid].page_table[y_page_num]*pagesize + y_offset;
                int z_main_mem_loc = pid_to_file[pid].page_table[z_page_num]*pagesize + z_offset;

                // Subtract the numbers, if not found, they are 0
                int diff = main_mem[x_main_mem_loc] - main_mem[y_main_mem_loc];
                if(verbose) cout << "Diff: " << diff << endl;
                main_mem[z_main_mem_loc] = diff;
                if(verbose) cout << "Value of addr " << x << " = " << main_mem[x_main_mem_loc] << ", Value of addr " << y << " = " << main_mem[y_main_mem_loc] << ", Value of addr " << z << " = " << main_mem[z_main_mem_loc] << endl;

                fout << "Result: Value of addr " << x << " = " << main_mem[x_main_mem_loc] << ", Value of addr " << y << " = " << main_mem[y_main_mem_loc] << ", Value of addr " << z << " = " << main_mem[z_main_mem_loc] << endl;
            }
            else if(first.compare("print")==0)
            {
                fout << "Command: " << "print " << line << endl;
                // Line: virtmemloc
                int virtmemloc = stoi(line);

                // Check if virtmemloc is valid
                if(virtmemloc > pid_to_file[pid].size*1024)
                {
                    if(verbose) cout << "Invalid virtual memory location" << endl;
                    fout << "Invalid virtual memory location" << endl;
                    continue;
                }

                // Convert virtmemloc to page number and offset
                int page_num = virtmemloc/pagesize;
                int offset = virtmemloc%pagesize;

                // Convert page number to main memory location
                int main_mem_loc = pid_to_file[pid].page_table[page_num]*pagesize + offset;
                if(verbose) cout << "Main mem loc: " << main_mem_loc << endl;

                // Print the value at virtmemloc
                fout << "Result: Value of addr " << virtmemloc << " = " << main_mem[main_mem_loc] << endl;
            }

        }
    }

    // Function to run the program
    void run()
    {
        // Empty the outfile
        ofstream fout(outfile);
        fout.close();

        ofstream fout_main(outfile, ios::app);

        // Read input file
        ifstream fin(infile);
        string line;
        while(getline(fin, line))
        {
            // Read first string
            string first = line.substr(0, line.find(" "));

            fout_main << ">> " << line << endl;

            if(verbose) cout << "Func called: " << first << endl;

            line = line.substr(line.find(" ")+1);
            
            // Compare first with "load", "run", "kill", "listpr", "pte", "pteall", "swapout", "swapin", "print", "exit"
            if(first.compare("load")==0)
            {
                // Load the process
                load(line);
            }
            else if(first.compare("run")==0)
            {
                // Run the process
                run_command(stoi(line));
            }
            else if(first.compare("kill")==0)
            {
                // Kill the process
                kill(stoi(line));
            }
            else if(first.compare("listpr")==0)
            {
                // List the processes
                listpr();
            }
            else if(first.compare("pte")==0)
            {
                // Print the page table entry
                pte(line, 1);
            }
            else if(first.compare("pteall")==0)
            {
                // Print all the page table entries
                pteall(line);
            }
            else if(first.compare("swapout")==0)
            {
                // Swap out the process
                swapout(stoi(line));
            }
            else if(first.compare("swapin")==0)
            {
                // Swap in the process
                swapin(stoi(line));
            }
            else if(first.compare("print")==0)
            {
                // Print the memory
                mem_print(line);
            }
            else if(first.compare("exit")==0)
            {
                // Exit the program
                exit(0);
            }
        }
    }

}; 

   

int main(int argc, char* argv[])
{
    // Take input for Main Mem Size(M), Virt Mem Size(V), Page Size(P), infile outfile using arguments
    int M = 8, V = 16, P = 512;
    string infile = "inp", outfile = "out";
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
        if(strcmp(argv[i], "-l")==0)
        {
            verbose = true;
        }
    }

    MEM mem = MEM(M, V, P, infile, outfile);
    mem.run();
}   