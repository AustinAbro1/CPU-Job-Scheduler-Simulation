#include <iostream>
#include <queue>
#include <fstream>
#include <string>
#include <sstream>
#include <array>
#include <vector>
#include <algorithm>

using namespace std;

ofstream outputFile("exampleOutput.txt");
ofstream logFile("exampleLog.txt");
ofstream visualization("exampleVisual.txt");

//5-states will be given to each process at a certain time (New, Ready, Running, Blocked, Exit)
enum State { New, Ready, Running, Blocked, Exit };

//This struct will be used to hold each process's attributes from the file
struct Process {
    int processId;
    int arrivalTime;
    int numberCpuBursts;
    int numberIoBursts;
    int totalWaitTime = 0;
    int cpuBursts[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    int indexCPU = 0;
    int ioBursts[10] = { -1,-1,-1,-1,-1,-1,-1,-1,-1,-1 };
    int indexIO = 0;
    State state = New;
    string stateDisplay = "New";
    int timeEnteredReadyQueue = 0;
    int timeEnteredIOQueue = 0;
    int timeSpentInState = 0;
    int timeExited = 0;

    //Metrics
    int turnAroundTime = 0;
    int readyWaitTime = 0;
    int ioWaitTime = 0;

};


double totalCPUTime;
bool cpuIdle = true;
bool ioIdle = true;
int timeQuantum = 0;
queue<Process> eventQueue; //Event queue for handling events in order of occurrence
queue<Process> readyQueue; //Ready queue for processes waiting for CPU
queue<Process> ioQueue; //I/O queue for processes waiting for I/O
queue<Process> allEventsExited; //Used for printing metrics




string cpuStatus = "CPU:";

string ioStatus = "IO:";

string rq = "RQ:";

string ioq = "IOQ:";

//This function will go through a vareity of invalid possibilies in a process and will detect it.
bool isValidProcess(Process pcb) {
    //Arrival time equals zero
    if (pcb.arrivalTime == 0) {
        outputFile << "Inavlid process, arrival time is less than 1" << endl;
        logFile << "Inavlid process, arrival time is less than 1" << endl;
        visualization << "Inavlid process, arrival time is less than 1" << endl;
        cout << "Inavlid process, arrival time is less than 1" << endl;
        return false;
    }
    //Number of CPU Bursts equals Zero
    else if (pcb.numberCpuBursts == 0) {
        outputFile << "Invalid process, number of cpu bursts is less than 1" << endl;
        logFile << "Invalid process, number of cpu bursts is less than 1" << endl;
        visualization << "Invalid process, number of cpu bursts is less than 1" << endl;
        cout << "Invalid process, number of cpu bursts is less than 1" << endl;
        return false;
    }
    //Wrong number of CPU bursts
    else if (pcb.cpuBursts[pcb.numberCpuBursts - 1] == -1) {
        outputFile << "Invalid process, total cpu bursts is not correct" << endl;
        logFile << "Invalid process, total cpu bursts is not correct" << endl;
        visualization << "Invalid process, total cpu bursts is not correct" << endl;
        cout << "Invalid process, total cpu bursts is not correct" << endl;
        return false;
    }
    //Finds if a burst is equal to zero
    else {
        for (int i = 0; i < 10; i++) {
            if (pcb.cpuBursts[i] == 0) {
                outputFile << "Invalid process, there is a cpu burst that is less than one" << endl;
                logFile << "Invalid process, there is a cpu burst that is less than one" << endl;
                visualization << "Invalid process, there is a cpu burst that is less than one" << endl;
                cout << "Invalid process, there is a cpu burst that is less than one" << endl;
                return false;
            }
            else if (pcb.ioBursts[i] == 0) {
                outputFile << "Invalid process, there is a io burst that is less than one" << endl;
                logFile << "Invalid process, there is a io burst that is less than one" << endl;
                visualization << "Invalid process, there is a io burst that is less than one" << endl;
                cout << "Invalid process, there is a io burst that is less than one" << endl;
                return false;
            }
        }
    }

    return true;
}

//This function reterives processes from a file for the simulation
void getJobsFromFile(string fileName, queue<Process>& eventQueue) {
    ifstream infile(fileName);
    //Checks if file exists
    if (!infile) {
        cout << "This file does not exist" << endl;
        outputFile << "This file does not exist" << endl;
        visualization << "This file does not exist" << endl;
        logFile << "This file does not exist" << endl;
    }
    string line;
    int process_id = 1;

    while (getline(infile, line)) {
        istringstream iss(line);
        Process pcb;

        pcb.processId = process_id;

        iss >> pcb.arrivalTime >> pcb.numberCpuBursts;

        int value;
        int counter = 0;

        //Differentiates between cpu and io bursts
        while (iss >> value) {
            if (counter % 2 == 0) {
                pcb.cpuBursts[pcb.indexCPU] = value;
                pcb.indexCPU++;
            }
            else {
                pcb.ioBursts[pcb.indexIO] = value;
                pcb.indexIO++;
            }
            counter++;
        }

        pcb.indexIO = 0;
        pcb.indexCPU = 0;

        //Checks if the process is valid before pushing it into the event queue
        if (isValidProcess(pcb)) {
            eventQueue.push(pcb);
            process_id++;
        }
    }
}

//function takes care of processes arriving
void arrival(Process& pcb, int simTime) {
    pcb.state = Ready;
    pcb.stateDisplay = "Ready";
    pcb.timeEnteredReadyQueue = simTime;
    readyQueue.push(pcb);
    rq += "(P" + to_string(pcb.processId) + ")";
    logFile << "Time:" << simTime << " Process ID " << pcb.processId << " arrived and it's state is " << pcb.stateDisplay << endl;
    cout << "Time:" << simTime << " Process ID " << pcb.processId << " arrived and it's state is " << pcb.stateDisplay << endl;
}

//function takes care of processes in the running state
void running(Process& pcb, int simTime, int timeQuantum) {
    totalCPUTime++;
    cpuIdle = false;
    pcb.state = Running;
    pcb.stateDisplay = "Running";
    pcb.cpuBursts[pcb.indexCPU] -= 1;
    logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    cpuStatus += "(P" + to_string(pcb.processId) + ") ";
    if (pcb.timeSpentInState == 0) {
        pcb.readyWaitTime += (simTime - pcb.timeEnteredReadyQueue);
    }
    ++pcb.timeSpentInState;

    //Checks to see if CPU burst equals zero to move it to the io queue and the end of the process has not been reached
    if (pcb.cpuBursts[pcb.indexCPU] == 0 && pcb.indexCPU + 1 != pcb.numberCpuBursts) {
        pcb.indexCPU++;
        pcb.state = Blocked;
        pcb.stateDisplay = "Blocked";
        pcb.timeEnteredIOQueue = simTime;
        pcb.timeSpentInState = 0;
        ioQueue.push(pcb);
        ioq += "(P" + to_string(pcb.processId) + ") ";
        cpuStatus += "(IQ)  ";
        cpuIdle = true;
        logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
        cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    }
    //Checks to see if CPU burst equals zero to move it to the io queue and the end of the process has been reached
    else if (pcb.cpuBursts[pcb.indexCPU] == 0 && pcb.indexCPU + 1 == pcb.numberCpuBursts) {
        pcb.state = Exit;
        pcb.stateDisplay = "Exit";
        pcb.timeSpentInState = 0;
        pcb.timeExited = simTime;
        pcb.turnAroundTime = pcb.timeExited - pcb.arrivalTime;
        allEventsExited.push(pcb);
        cpuIdle = true;
        logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
        cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    }
    //Checks to see if time quantum has been reached
    else if (pcb.timeSpentInState == timeQuantum) {
        pcb.state = Ready;
        pcb.stateDisplay = "Ready";
        pcb.timeEnteredReadyQueue = simTime;
        pcb.timeSpentInState = 0;
        readyQueue.push(pcb);
        rq += "(P" + to_string(pcb.processId) + ") ";
        cpuStatus += "(RQ)  ";
        cpuIdle = true;
        logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
        cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    }
}

//function takes care of processes in the blocked state
void blocked(Process& pcb, int simTime, int timeQuantum) {
    ioIdle = false;
    pcb.state = Blocked;
    pcb.stateDisplay = "Blocked";
    pcb.ioBursts[pcb.indexIO] -= 1;
    logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    ioStatus += "(P" + to_string(pcb.processId) + ") ";
    if (pcb.timeSpentInState == 0) {
        pcb.ioWaitTime += (simTime - pcb.timeEnteredIOQueue);
    }
    ++pcb.timeSpentInState;

    //Checks to see if IO burst equals zero to move it to the io queue
    if (pcb.ioBursts[pcb.indexIO] == 0) {
        pcb.indexIO++;
        pcb.state = Ready;
        pcb.stateDisplay = "Ready";
        pcb.timeEnteredReadyQueue = simTime;
        pcb.timeSpentInState = 0;
        readyQueue.push(pcb);
        rq += "(P" + to_string(pcb.processId) + ") ";
        ioStatus += "(RQ)  ";
        ioIdle = true;
        logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
        cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    }
    //Checks to see if time quantum has been reached
    else if (pcb.timeSpentInState == timeQuantum) {
        pcb.state = Blocked;
        pcb.stateDisplay = "Blocked";
        pcb.timeEnteredIOQueue = simTime;
        pcb.timeSpentInState = 0;
        ioQueue.push(pcb);
        ioq += "(P" + to_string(pcb.processId) + ") ";
        ioStatus += "(IQ)  ";
        ioIdle = true;
        logFile << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
        cout << "Time:" << simTime << " Process ID " << pcb.processId << "'s state is " << pcb.stateDisplay << endl;
    }
}

int main() {
    //Takes input from file
    string inputFile;
    cout << "Please choose file input name: ";
    cin >> inputFile;
    cout << endl;


    getJobsFromFile(inputFile, eventQueue);

    int simTime = 1; //Current simulation time


    cout << "Choose a quantum time number: ";
    cin >> timeQuantum;
    cout << endl;

    //Checks for invalid quantum time input
    if (timeQuantum <= 0) {
        cout << "Inavlid quantum time number." << endl;
        outputFile << "Inavlid quantum time number." << endl;
        logFile << "Inavlid quantum time number." << endl;
        visualization << "Inavlid quantum time number." << endl;
        return 0;
    }
    Process proc;
    Process proc2;
    Process proc3;

    //Runs while all queues are not empty, and both the cpu and io are not idle
    while (!eventQueue.empty() || !cpuIdle || !ioIdle || !ioQueue.empty() || !readyQueue.empty()) {
        if (!eventQueue.empty()) {
            proc = eventQueue.front();
        }
        //For visual
        cpuStatus += " " + to_string(simTime) + ": ";
        ioStatus += " " + to_string(simTime) + ": ";
        rq += " " + to_string(simTime) + ": ";
        ioq += " " + to_string(simTime) + ": ";

        if (proc.arrivalTime != simTime && cpuIdle && ioIdle && readyQueue.empty() && ioQueue.empty()) {
            logFile << "Time:" << simTime << " No events happening in the simulation" << endl;
            cout << "Time:" << simTime << " No events happening in the simulation" << endl;
        }
        //If arrival time equals sim time, call the arrival function
        else if (proc.arrivalTime == simTime) {
            arrival(proc, simTime);

            eventQueue.pop();
        }
        //if cpu is idle and the rq is not empty, call the running function and set it to the front of the ready queue
        if (cpuIdle && !readyQueue.empty()) {
            proc2 = readyQueue.front();
            running(proc2,simTime,timeQuantum);
            readyQueue.pop();
        }
        //else if cpu is not idle, keep running the same process stored in process 2
        else if (!cpuIdle) {
            running(proc2,simTime,timeQuantum);
        }
        //if io is idle and the ioq is not empty, call the running function and set it to the front of the io queue
        if (ioIdle && !ioQueue.empty()) {
            proc3 = ioQueue.front();
            blocked(proc3, simTime, timeQuantum);
            ioQueue.pop();
        }
        //else if io is not idle, keep running the same process stored in process 3
        else if (!ioIdle) {
            blocked(proc3, simTime, timeQuantum);
        }

        simTime++;
    }

    //Calculation for cpu utilzation
    double cpuUtilization = (totalCPUTime / simTime) * 100;


    cout << endl;
    outputFile << "Processes may not be listed in exact numerical order " << endl;
    cout << "Processes may not be listed in exact numerical order." << endl;
    outputFile << "Results for quantum: " << timeQuantum << " CPU Utilization = " << cpuUtilization << "%" << endl;
    cout << "Results for quantum: " << timeQuantum << " CPU Utilization = " << cpuUtilization << "%" << endl;
    cout << endl;

    Process procMetrics;
    while (!allEventsExited.empty()) {
        procMetrics = allEventsExited.front();
        cout << "P" << procMetrics.processId << " (" << "Turn Around Time = " << procMetrics.turnAroundTime << " Ready Wait Time = " 
            << procMetrics.readyWaitTime << " I/O Wait Time = " << procMetrics.ioWaitTime << ")" << endl;
        outputFile << "P" << procMetrics.processId << " (" << "Turn Around Time = " << procMetrics.turnAroundTime << " Ready Wait Time = "
            << procMetrics.readyWaitTime << " I/O Wait Time = " << procMetrics.ioWaitTime << ")" << endl;
        allEventsExited.pop();
    }

    visualization << "Visualization IQ – move to I / O Queue RQ – move to Ready Queue" << endl;
    visualization << "Processes and RQ/IOQ Switches are in parenthesis... example - (P1)" << endl;
    visualization << endl;

    visualization << cpuStatus << endl;
    visualization << endl;

    visualization << ioStatus << endl;
    visualization << endl;

    visualization << rq << endl;
    visualization << endl;

    visualization << ioq << endl;
    visualization << endl;

    //Console Visualization
    cout << "Visualization IQ: move to I / O Queue RQ: move to Ready Queue" << endl;
    cout << "Processes and RQ/IOQ Switches are in parenthesis... example - (P1)" << endl;
    cout << endl;

    cout << cpuStatus << endl;
    cout << endl;

    cout << ioStatus << endl;
    cout << endl;

    cout << rq << endl;
    cout << endl;

    cout << ioq << endl;
    cout << endl;


}