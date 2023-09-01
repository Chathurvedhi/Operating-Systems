#include<bits/stdc++.h>

using namespace std;

struct ComparePairs {
    bool operator()(const pair<int, int>& a, const pair<int, int>& b) {
        if (a.first == b.first) {
            return a.second > b.second; // Compare second element if first elements are equal
        }
        return a.first > b.first; // Compare first element
    }
};

class Process
{
    public:
    int id;                 // process id
    int arrival_time;       // arrival time of the process
    int burst_time;         // burst time of the process
    int completion_time;    // completion time of the process
    int process_time;       // time spent in the CPU

    int start_level;        // level at which the process starts
    int end_level;          // level at which the process ends
    int prio_jumps;         // number of times the process jumps priority
    bool finished;          // flag to check if the process is finished
};

class MLFQ
{
    int RR_slice;           // time slice for round robin
    int prio_jump_timer;    // time after which the priority jumps
    int num_proc;           // number of processes
    int num_proc_finished;  // number of processes finished
    int current_time;       // current time
    int RR_time;            // timer for round robin

    string input_file;      // input file name
    string output_file;     // output file name
    bool verbose;           // flag to check if verbose output is needed

    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> proc_start;     // stores the processes in the order of arrival time
    unordered_map<int, Process> processes;  // stores the processes with their id as key
    queue<int> RR_4, FCFS_1;                
    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> SJF_3, SJF_2;

    // Constructor
    public:
    MLFQ(int Q, int T, string input_file, string output_file, bool V)
    {
        this->RR_slice = Q;
        this->prio_jump_timer = T;
        this->input_file = input_file;
        this->output_file = output_file;
        this->verbose = V;
    }

    // Function to read the input file
    void read_input()
    {
        ifstream fin(input_file);
        num_proc = 0;
        int id, start_level, arrival_time, burst_time;
        while(fin>>id>>start_level>>arrival_time>>burst_time)
        {
            Process p;
            p.id = id;
            p.start_level = start_level;
            p.arrival_time = arrival_time;
            p.burst_time = burst_time;
            p.completion_time = -1;
            p.process_time = 0;
            p.end_level = -1;
            p.prio_jumps = 1;
            p.finished = false;
            processes[id] = p;
            num_proc++;
        }
        fin.close();

        for(auto it=processes.begin(); it!=processes.end(); it++)
        {
            proc_start.push({it->second.arrival_time, it->second.id});
        }
    }

    // Function for final output
    void output_stats()
    {
        ofstream fout(output_file);
        int tat_avg = 0;
        for(int i=1; i<=num_proc; i++)
        {
            fout << "ID :" << processes[i].id << "; Orig Level: " << processes[i].start_level << "; Final Level: " << processes[i].end_level << "; Comp Time: " << processes[i].completion_time << "; TAT: " << processes[i].completion_time - processes[i].arrival_time << endl;
            tat_avg += processes[i].completion_time - processes[i].arrival_time;
        }
        fout << "Mean Turnaround Time: " << tat_avg/num_proc << "; ";
        fout << "Throughput " << num_proc/(current_time*1.0) << endl;
        fout.close();
    }

    // Function for verbose output
    void print_stats()
    {
        cout << "***********************" << endl;
        cout << "Current Time " << current_time << endl;

        cout << "RR_4 Queue: ";
        queue<int> temp = RR_4;
        while(!temp.empty())
        {
            cout << "[" << temp.front() << ", " << processes[temp.front()].process_time << ", " << processes[temp.front()].burst_time << ", " << processes[temp.front()].arrival_time << "] ;";
            temp.pop();
        }
        cout << endl;

        cout << "SJF_3 Queue: ";
        priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> temp1 = SJF_3;
        while(!temp1.empty())
        {
            cout << "[" << temp1.top().second << ", " << processes[temp1.top().second].process_time << ", " << processes[temp1.top().second].burst_time << ", " << processes[temp1.top().second].arrival_time << "] ; ";
            temp1.pop();
        }
        cout << endl;

        cout << "SJF_2 Queue: ";
        priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> temp2 = SJF_2;
        while(!temp2.empty())
        {
            cout << "[" << temp2.top().second << ", " << processes[temp2.top().second].process_time << ", " << processes[temp2.top().second].burst_time << ", " << processes[temp2.top().second].arrival_time << "] ; ";
            temp2.pop();
        }
        cout << endl;

        cout << "FCFS_1 Queue: ";
        queue<int> temp3 = FCFS_1;
        while(!temp3.empty())
        {
            cout << "[" << temp3.front() << ", " << processes[temp3.front()].process_time << ", " << processes[temp3.front()].burst_time << ", " << processes[temp3.front()].arrival_time << "] ; ";
            temp3.pop();
        }
        cout << endl;
    }

    void proc_add()
    {
        priority_queue<int, vector<int>, greater<int>> temp_RR_4;
        priority_queue<int, vector<int>, greater<int>> temp_FCFS_1;
        vector<int> temp_SJF_3, temp_SJF_2;

        // Arrivals to MLFQ
        while(!proc_start.empty() && proc_start.top().first <= current_time)
        {
            int id = proc_start.top().second;
            proc_start.pop();
            if(processes[id].start_level == 4)
            {
                temp_RR_4.push(id);
            }
            else if(processes[id].start_level == 3)
            {
                temp_SJF_3.push_back(id);
            }
            else if(processes[id].start_level == 2)
            {
                temp_SJF_2.push_back(id);
            }
            else if(processes[id].start_level == 1)
            {
                temp_FCFS_1.push(id);
            }
        }

        // Priority jumps

        vector<pair<int, int>> temp;

        // Check for SJF_3 queue and promote to RR_4
        while(!SJF_3.empty())
        {
            temp.push_back(SJF_3.top());
            SJF_3.pop();
        }

        for(auto it=temp.begin(); it!=temp.end(); it++)
        {
            int id = it->second;
            if(processes[id].prio_jumps * prio_jump_timer + processes[id].arrival_time <= current_time)
            {
                processes[id].prio_jumps++;
                temp_RR_4.push(id);
                temp.erase(it);
                it--;
            }

            else
            {
                SJF_3.push(*it);
            }
        }

        // Check for SJF_2 queue and promote to SJF_3
        temp.clear();

        while(!SJF_2.empty())
        {
            temp.push_back(SJF_2.top());
            SJF_2.pop();
        }

        for(auto it=temp.begin(); it!=temp.end(); it++)
        {
            int id = it->second;
            if(processes[id].prio_jumps * prio_jump_timer + processes[id].arrival_time <= current_time)
            {
                processes[id].prio_jumps++;
                temp_SJF_3.push_back(id);
                temp.erase(it);
                it--;
            }

            else
            {
                SJF_2.push(*it);
            }
        }

        // Check for FCFS_1 queue and promote to SJF_2
        while(!FCFS_1.empty())
        {
            int id = FCFS_1.front();
            if(processes[id].prio_jumps * prio_jump_timer + processes[id].arrival_time <= current_time)
            {
                processes[id].prio_jumps++;
                temp_SJF_2.push_back(id);
                FCFS_1.pop();
            }
            else
            {
                break;
            }   
        }

        // Add the processes to the queues

        while(!temp_RR_4.empty())
        {
            RR_4.push(temp_RR_4.top());
            temp_RR_4.pop();
        }

        for(auto it=temp_SJF_3.begin(); it!=temp_SJF_3.end(); it++)
        {
            SJF_3.push({processes[*it].burst_time, *it});
        }

        for(auto it=temp_SJF_2.begin(); it!=temp_SJF_2.end(); it++)
        {
            SJF_2.push({processes[*it].burst_time, *it});
        }

        while(!temp_FCFS_1.empty())
        {
            FCFS_1.push(temp_FCFS_1.top());
            temp_FCFS_1.pop();
        }
    }

    // Function to run the processes in RR_4 queue for 1 unit of time
    void run_RR()
    {
        int id = RR_4.front();
        processes[id].process_time++;
        RR_time++;
        current_time++;

        if(processes[id].process_time == processes[id].burst_time)
        {
            RR_4.pop();
            processes[id].completion_time = current_time;
            processes[id].end_level = 4;
            processes[id].finished = true;
            num_proc_finished++;
            RR_time = 0;
        }

        else if(RR_time == RR_slice)
        {
            RR_4.pop();
            RR_4.push(id);
            RR_time = 0;
        }
    }

    // Function to run the processes in SJF_3 queue for 1 unit of time
    void run_SJF_3()
    {
        int id = SJF_3.top().second;
        processes[id].process_time++;
        current_time++;

        if(processes[id].process_time == processes[id].burst_time)
        {
            processes[id].completion_time = current_time;
            processes[id].end_level = 3;
            processes[id].finished = true;
            num_proc_finished++;
            SJF_3.pop();
        }
    }

    // Function to run the processes in SJF_2 queue for 1 unit of time
    void run_SJF_2()
    {
        int id = SJF_2.top().second;
        processes[id].process_time++;
        current_time++;

        if(processes[id].process_time == processes[id].burst_time)
        {
            processes[id].completion_time = current_time;
            processes[id].end_level = 2;
            processes[id].finished = true;
            num_proc_finished++;
            SJF_2.pop();
        }
    }

    // Function to run the processes in FCFS_1 queue for 1 unit of time
    void run_FCFS()
    {
        int id = FCFS_1.front();
        processes[id].process_time++;
        current_time++;

        if(processes[id].process_time == processes[id].burst_time)
        {
            processes[id].completion_time = current_time;
            processes[id].end_level = 1;
            processes[id].finished = true;
            num_proc_finished++;
            FCFS_1.pop();
        }
    }

    // Main run
    void main_run()
    {
        read_input();
        current_time = 0;
        num_proc_finished = 0;

        while(num_proc_finished != num_proc)
        {
            // Add processes via arrival or priority jump
            proc_add();

            // Stats printing if needed
            if(verbose) print_stats();

            // Run the processes based on highest priority non empty queue
            if(!RR_4.empty())
            {
                run_RR();
            }
            else if(!SJF_3.empty())
            {
                run_SJF_3();
            }
            else if(!SJF_2.empty())
            {
                run_SJF_2();
            }
            else if(!FCFS_1.empty())
            {
                run_FCFS();
            }
            else
            {
                current_time++;
            }
        }

        // Output to file
        output_stats();
    }
};

int main(int argc, char ** argv ) 
{
    int Q = 10, T = 500;
    bool V = false;
    string input_file = "input.txt", output_file = "output.txt";

    
    for(int i=1; i<argc; i++)
    {
        if(strcmp(argv[i], "-Q")==0)
        {
            Q = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-T")==0)
        {
            T = atoi(argv[i+1]);
            i++;
        }
        else if(strcmp(argv[i], "-F")==0)
        {
            input_file = argv[i+1];
            i++;
        }
        else if(strcmp(argv[i], "-P")==0)
        {
            output_file = argv[i+1];
            i++;
        }
        else if(strcmp(argv[i], "-v")==0)
        {
            V = true;
        }
    }

    MLFQ Queue_Run = MLFQ(Q, T, input_file, output_file, V);

    // MLFQ Queue_Run = MLFQ(10, 50, "input.txt", "output.txt");
    Queue_Run.main_run();
}