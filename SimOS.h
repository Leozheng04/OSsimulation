// Name: Leo Zheng 

#ifndef SIMOS_H
#define SIMOS_H

#include <string>
#include <deque>
#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <stack>
#include <algorithm>

struct FileReadRequest{
    int PID{0};
    std::string fileName{""};
};
struct MemoryItem{
    unsigned long long pageNumber;
    unsigned long long frameNumber;
    int PID; // PID of the process using this frame of memory
};

using MemoryUsage = std::vector<MemoryItem>;
constexpr int NO_PROCESS{ 0 };

class SimOS{
    private:
        int numberOfDisks_;
        unsigned long long amountOfRAM_;
        unsigned int pageSize_;
        //Process
        struct Process{
            int PID;
            int parentPID;
            std::vector<int> children;
            bool isZombie{false};
            bool isWaiting{false};
        };
        // Needed to simulate the CPU
        int cpu_{NO_PROCESS};
        int nextPID_{1};
        std::deque<int> readyQueue_;
        std::unordered_map<int,Process> processes_;
        //Disk
        struct Disk{
            FileReadRequest currServing;
            std::deque<FileReadRequest> diskQueue;
        };
        std::unordered_map<int,Disk> disks_;
        //Frame
        unsigned long long totalFrames_;
        std::vector<MemoryItem> frames_;
        std::vector<unsigned long long> emptyFrames_;
        // int for the PID and unsigned long long for page number
        std::deque<std::pair<int, unsigned long long>> lruOrder_; 

        //Helper function(s)
        void checkCpuState();
        void checkDiskState(int diskNumber);
        void casacadeTermination(int PID);
    public:
        // The parameters specify number of hard disks in the simulated computer, amount of memory, and page size.
        // Disks, frame, and page enumerations all start from 0.
        SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize);

        // Creates a new process in the simulated system. The new process takes place in the ready-queue or immediately starts using the CPU.
        // Every process in the simulated system has a PID. Your simulation assigns PIDs to new processes starting from 1 and increments it by one for each new process. Do not reuse PIDs of the terminated processes.
        void NewProcess();

        // The currently running process forks a child. The child is placed in the end of the ready-queue.
        void SimFork();

        // The process that is currently using the CPU terminates. Make sure you release the memory used by this process immediately. If its parent is already waiting, the process terminates immediately and the parent becomes runnable (goes to the ready-queue). 
        // If its parent hasn't called wait yet, the process turns into zombie.
        // To avoid the appearance of the orphans, the system implements the cascading termination. 
        // Cascading termination means that if a process terminates, all its descendants terminate with it.
        void SimExit();

        // The process wants to pause and wait for any of its child processes to terminate. Once the wait is over, the process goes to the end of the ready-queue or the CPU. If the zombie-child already exists, the process proceeds right away (keeps using the CPU) and the zombie-child disappears. If more than one zombie-child exists, the system uses one of them (any!) to immediately resumes the parent, while other zombies keep waiting for the next wait from the parent.
        void SimWait();       

        // Interrupt arrives from the timer signaling that the time slice of the currently running process is over.
        void TimerInterrupt();

        // Currently running process requests to read the specified file from the disk with a given number. The process issuing disk reading requests immediately stops using the CPU, even if the ready-queue is empty. The process is served by the specified disk and placed in the end of the disk's I/O-queue. When the disk finishes reading the file, the process is moved to the end of the ready-queue.
        void DiskReadRequest( int diskNumber, std::string fileName );

        // A disk with a specified number reports that a single job is completed. The served process should return to the ready-queue.
        void DiskJobCompleted( int diskNumber );

        // Currently running process wants to access the specified logical memory address. System makes sure the corresponding page is loaded in the RAM. If the corresponding page is already in the RAM, its “recently used” information is updated. When multiple frames are available, the system chooses the one with the lower number.
        void AccessMemoryAddress(unsigned long long address);

        // GetCPU returns the PID of the process currently using the CPU. If CPU is idle it returns NO_PROCESS (see the supplied definitions above).
        int GetCPU();

        // GetReadyQueue returns the std::deque with PIDs of processes in the ready-queue where element in front corresponds start of the ready-queue. The currently running process doesn't appear in the ready-queue.
        std::deque<int> GetReadyQueue();

        // GetMemory returns MemoryUsage vector describing all currently used frames of RAM. Remember, terminated “zombie” processes don’t use memory, so they don’t contribute to memory usage.
        // MemoryItems appear in the MemoryUsage vector in the order from low frame number to high.
        MemoryUsage GetMemory();

        // GetDisk returns an object with PID of the process served by specified disk and the name of the file read for that process. If the disk is idle, GetDisk returns the default FileReadRequest object (with PID 0 and empty string in fileName) 
        FileReadRequest GetDisk( int diskNumber );

        // GetDiskQueue returns the I/O-queue of the specified disk starting from the “next to be served” process. The currently served process doesn't appear in this queue.
        std::deque<FileReadRequest> GetDiskQueue( int diskNumber );

};

#endif // SIMOS_H