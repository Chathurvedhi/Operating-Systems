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

    string input_file;      // input file name
    string output_file;     // output file name

    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> proc_start;     // stores the processes in the order of arrival time
    unordered_map<int, Process> processes;  // stores the processes with their id as key
    queue<int> RR_4, FCFS_1;                
    priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> SJF_3, SJF_2;

    // Constructor
    public:
    MLFQ(int Q, int T, string input_file, string output_file)
    {
        this->RR_slice = Q;
        this->prio_jump_timer = T;
        this->input_file = input_file;
        this->output_file = output_file;
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

        // Add processes to priority queue in the order of arrival time and then id
        for(auto it=processes.begin(); it!=processes.end(); it++)
        {
            proc_start.push({it->second.arrival_time, it->second.id});
        }

        fin.close();
    }

    // Check for priority jumps
    void check_prio_switch()
    {
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
                RR_4.push(id);
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
                SJF_3.push(*it);
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
                SJF_2.push({processes[id].burst_time, id});
                FCFS_1.pop();
            }
            else
            {
                break;
            }   
        }
    }

    // RR_4 queue iteration
    void RR_4_run()
    {
        int id = RR_4.front();
        RR_4.pop();
        if(RR_slice > processes[id].burst_time - processes[id].process_time)
        {
            current_time += processes[id].burst_time - processes[id].process_time;
            processes[id].process_time = processes[id].burst_time;
            processes[id].completion_time = current_time;
            processes[id].finished = true;
            processes[id].end_level = 4;
            num_proc_finished++;
        }
        else
        {
            processes[id].process_time += RR_slice;
            current_time += RR_slice;
            RR_4.push(id);
        }   
    }

    // SJF_3 iteration
    void SJF_3_run()
    {
        int id = SJF_3.top().second;
        SJF_3.pop();
        current_time = current_time + processes[id].burst_time - processes[id].process_time;
        processes[id].process_time = processes[id].burst_time;
        processes[id].completion_time = current_time;
        processes[id].end_level = 3;
        processes[id].finished = true;
        num_proc_finished++;
    }

    // SJF_2 iteration
    void SJF_2_run()
    {
        int id = SJF_2.top().second;
        SJF_2.pop();
        current_time = current_time + processes[id].burst_time - processes[id].process_time;
        processes[id].process_time = processes[id].burst_time;
        processes[id].completion_time = current_time;
        processes[id].end_level = 2;
        processes[id].finished = true;
        num_proc_finished++;
    }

    // FCFS_1 iteration
    void FCFS_1_run()
    {
        int id = FCFS_1.front();
        FCFS_1.pop();
        current_time = current_time + processes[id].burst_time - processes[id].process_time;
        processes[id].process_time = processes[id].burst_time;
        processes[id].completion_time = current_time;
        processes[id].end_level = 1;
        processes[id].finished = true;
        num_proc_finished++;
    }

    // Final Output function
    void output_stats()
    {
        ofstream fout(output_file);
        int tat_avg = 0;
        for(int i=1; i<=num_proc; i++)
        {
            fout << "ID :" << processes[i].id << " Orig Level " << processes[i].start_level << " Final Level " << processes[i].end_level << " Comp Time " << processes[i].completion_time << " TAT " << processes[i].completion_time - processes[i].arrival_time << endl;
            tat_avg += processes[i].completion_time - processes[i].arrival_time;
        }
        fout << "Average TAT " << tat_avg/num_proc << endl;
        fout << "Throughput " << num_proc/(current_time*1.0) << endl;
        fout.close();
    }

    // Print the current state of the queues
    void print_stats()
    {
        // Print current time
        cout << "Current time: " << current_time << endl;
        cout << "------------------------------" << endl;

        // Print all queues
        cout << "RR_4: ";
        queue<int> temp_rr = RR_4;
        while(!temp_rr.empty())
        {
            cout << temp_rr.front() << " ";
            temp_rr.pop();
        }
        cout << endl;

        priority_queue<pair<int, int>, vector<pair<int, int>>, ComparePairs> temp;
        cout << "SJF_3: ";
        temp = SJF_3;
        while(!temp.empty())
        {
            cout << temp.top().second << " ";
            temp.pop();
        }
        cout << endl;

        cout << "SJF_2: ";
        temp = SJF_2;
        while(!temp.empty())
        {
            cout << temp.top().second << " ";
            temp.pop();
        }
        cout << endl;

        cout << "FCFS_1: ";
        temp_rr = FCFS_1;
        while(!temp.empty())
        {
            cout << temp_rr.front() << " ";
            temp.pop();
        }
        cout << endl;
        cout << "------------------------------" << endl;
    }

    void main_run()
    {
        current_time = 0;
        num_proc_finished = 0;

        // Read the input file
        read_input();

        // Run the scheduler
        while(num_proc != num_proc_finished)
        {
            // Print the current state of the queues
            // print_stats();

            // Check for additions to the queues
            while(!proc_start.empty() && proc_start.top().first <= current_time)
            {
                int id = proc_start.top().second;
                proc_start.pop();
                if(processes[id].start_level == 4)
                {
                    RR_4.push(id);
                }
                else if(processes[id].start_level == 3)
                {
                    SJF_3.push({processes[id].burst_time, id});
                }
                else if(processes[id].start_level == 2)
                {
                    SJF_2.push({processes[id].burst_time, id});
                }
                else if(processes[id].start_level == 1)
                {
                    FCFS_1.push(id);
                }
            }

            // Print the current state of the queues
            // print_stats();

            // Check for priority jumps
            check_prio_switch();

            // Check if all queues are empty
            if(RR_4.empty() && SJF_3.empty() && SJF_2.empty() && FCFS_1.empty())
            {
                current_time = proc_start.top().first;
                continue;
            }

            // Run the queues
            if(!RR_4.empty())
            {
                RR_4_run();
            }
            else if(!SJF_3.empty())
            {
                SJF_3_run();
            }
            else if(!SJF_2.empty())
            {
                SJF_2_run();
            }
            else if(!FCFS_1.empty())
            {
                FCFS_1_run();
            }

            // Print the current state of the queues
            // print_stats();
        }

        // Print the final output
        output_stats();
    }
};

int main(int argc, char ** argv ) 
{
    int Q = 10, T = 50;
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
    }

    MLFQ Queue_Run = MLFQ(Q, T, input_file, output_file);

    // MLFQ Queue_Run = MLFQ(10, 50, "input.txt", "output.txt");
    Queue_Run.main_run();
}