// Implement MLFQ with 4 queues(RR, SJF, SJF, FCFS) in priority order higher to lower.
// RR queue has quantum Q(input).
// Priority boost after T(input) time units by one level.
// Input from file, each line has id, start level, start time, CPU burst time

// Command line arguments for Q, T, input file name(-F), output file name(-P) with flags

#include<bits/stdc++.h>

using namespace std;

class Process
{
    public:
    int id;
    int start_level;
    int start_time;
    int burst_time;
    int prio_time;
    int final_level;
    int run_time;
    int end_time;
    bool finished;
};

class MLFQ
{
    int RR_slice, priority_time, num_process, num_process_completed, current_time;
    bool all_queue_empty;
    string input_file, output_file;
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> proc_start;
    unordered_map<int, Process> processes;
    queue<int> RR_4, FCFS_1;
    vector<pair<int, int>> SJF_2, SJF_3;

    // Constructor
    public:
    MLFQ(int Q, int T, string input_file, string output_file)
    {
        this->RR_slice = Q;
        this->priority_time = T;
        this->input_file = input_file;
        this->output_file = output_file;
    }

    // Function to read input from file
    void read_input()
    {
        ifstream fin(input_file);
        int id, start_level, start_time, burst_time;
        while(fin>>id>>start_level>>start_time>>burst_time)
        {
            Process p;
            p.id = id;
            p.start_level = start_level;
            p.start_time = start_time;
            p.prio_time = start_time;
            p.burst_time = burst_time;
            p.final_level = start_level;
            p.run_time = 0;
            p.end_time = -1;
            p.finished = false;
            processes[id] = p;
            num_process++;

            proc_start.push({start_time, id});
        }
        fin.close();
    }  

    // Function to run the MLFQ
    void run()
    {
        current_time = 0;
        num_process_completed = 0;
        read_input();
        all_queue_empty = false;

        while(num_process != num_process_completed)
        {
            //Check if any process can be added to any queue
            while(!proc_start.empty() && proc_start.top().first <= current_time)
            {
                int id = proc_start.top().second;
                proc_start.pop();
                if(processes[id].start_level == 1)
                    FCFS_1.push(id);
                else if(processes[id].start_level == 2)
                    SJF_2.emplace_back(make_pair(processes[id].burst_time, id));
                else if(processes[id].start_level == 3)
                    SJF_3.emplace_back(make_pair(processes[id].burst_time, id));
                else if(processes[id].start_level == 4)
                    RR_4.push(id);
            }

            // Check all processes in SJF_3 queue and promote to RR_4
            for(int i=0; i<SJF_3.size(); i++)
            {
                int id = SJF_3[i].second;
                if(processes[id].prio_time + priority_time <= current_time)
                {
                    processes[id].prio_time = current_time;
                    processes[id].final_level = 4;
                    RR_4.push(id);
                    SJF_3.erase(SJF_3.begin() + i);
                    i--;
                }
            }

            // Check all processes in SJF_2 queue and promote to SJF_3
            for(int i=0; i<SJF_2.size(); i++)
            {
                int id = SJF_2[i].second;
                if(processes[id].prio_time + priority_time <= current_time)
                {
                    processes[id].prio_time = current_time;
                    processes[id].final_level = 3;
                    SJF_3.emplace_back(make_pair(processes[id].burst_time, id));
                    SJF_2.erase(SJF_2.begin() + i);
                    i--;
                }
            }

            // Check all processes in FCFS_1 queue and promote to SJF_2
            while(!FCFS_1.empty())
            {
                int id = FCFS_1.front();
                if(processes[id].prio_time + priority_time <= current_time)
                {
                    processes[id].prio_time = current_time;
                    processes[id].final_level = 2;
                    SJF_2.emplace_back(make_pair(processes[id].burst_time, id));
                    FCFS_1.pop();
                }
                else
                    break;
            }

            // RR_4 queue iteration if not empty
            
        }
    }
};

int main(int argc, char ** argv ) 
{
    int Q, T;
    string input_file, output_file;
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
    }

     MLFQ Queue_Run = MLFQ(Q, T, input_file, output_file);
     Queue_Run.run();
}