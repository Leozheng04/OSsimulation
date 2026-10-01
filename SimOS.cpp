#include "SimOS.h"
#include <stdexcept>

void SimOS::checkCpuState(){
    if (!cpu_){
        throw std::logic_error("CPU is idle");
    }  
}
void SimOS::checkDiskState(int diskNumber){
    if (disks_.find(diskNumber) == disks_.end()) {
        throw std::out_of_range("disk with the requested number doesn't exist");
    }
}

void SimOS::casacadeTermination(int PID){
    std::stack<int> s;
    s.push(PID);
    while (!s.empty()){
        int currPID = s.top();
        s.pop();
        for (const auto& child: processes_[currPID].children){
            s.push(child);
        }
        for (int i = 0; i < readyQueue_.size(); i++){
            if (readyQueue_[i] == currPID){
                readyQueue_.erase(readyQueue_.begin() + i);
                break;
            }
        }
        for (int i = 0; i < frames_.size(); i++){
        if (frames_[i].PID == currPID){
            emptyFrames_.push_back(frames_[i].frameNumber);
            frames_.erase(frames_.begin() + i);
            i--;
        }
        }
        for (int i = 0; i < lruOrder_.size(); i++){
            if (lruOrder_[i].first == currPID){
                lruOrder_.erase(lruOrder_.begin() + i);
                i--;
            }
        }
        std::sort(emptyFrames_.begin(), emptyFrames_.end());
            processes_.erase(currPID);
        }
}

SimOS::SimOS( int numberOfDisks, unsigned long long amountOfRAM, unsigned int pageSize):
    numberOfDisks_(numberOfDisks), amountOfRAM_(amountOfRAM), pageSize_(pageSize),
    cpu_(NO_PROCESS), nextPID_(1), totalFrames_(pageSize == 0 ? 0: amountOfRAM / pageSize) {
        Disk disk;
        for (int i = 0; i < numberOfDisks_; i++){
            disks_[i] = disk;
        }
        for (unsigned long long i = 0; i < totalFrames_; i++){
            emptyFrames_.push_back(i);
        }
}

void SimOS::NewProcess(){
    Process newProcess;
    newProcess.PID = nextPID_;
    newProcess.parentPID = NO_PROCESS;
    if (!cpu_){
        cpu_ = newProcess.PID;
    }
    else{
        readyQueue_.push_back(newProcess.PID);
    }
    processes_[newProcess.PID]= newProcess;
    nextPID_++;
}

void SimOS::SimFork(){
    checkCpuState();
    Process child; 
    child.PID = nextPID_;
    child.parentPID = cpu_;
    // Put child in process vector
    processes_[child.PID] = child;
    // Update the parent process to remember it's child
    processes_[cpu_].children.push_back(child.PID);
    readyQueue_.push_back(child.PID);
    nextPID_++;
}

void SimOS::SimExit(){
    checkCpuState();
    int currProcess = cpu_;

    for (const auto& child: processes_[currProcess].children){
        casacadeTermination(child);
    }
    // Double checking if process actually exist
    // Might be unnessacry but just in case
    if (processes_.find(cpu_) == processes_.end()){
        return;
    }
    for (int i = 0; i < frames_.size(); i++){
        if (frames_[i].PID == currProcess){
            emptyFrames_.push_back(frames_[i].frameNumber);
            frames_.erase(frames_.begin() + i);
            i--;
        }
    }
    for (int i = 0; i < lruOrder_.size(); i++){
        if (lruOrder_[i].first == currProcess){
            lruOrder_.erase(lruOrder_.begin() + i);
            i--;
        }
    }
    std::sort(emptyFrames_.begin(), emptyFrames_.end());
    // Edge Case: Dealing with Zombie Process
    int parent = processes_[currProcess].parentPID;
    // 1 from the spec) If the parent exist and is not waiting. Turn child into zombie 
    if (parent != NO_PROCESS && !processes_[parent].isWaiting){
        processes_[currProcess].isZombie = true;
    }
    //2 now the case when the parent exist but is waiting. Then we can just terminate the process.
    // And put the parent back in the ready queue, also remove the child cause it is not there anymore
    else if (parent != NO_PROCESS && processes_[parent].isWaiting){
        processes_[parent].isWaiting = false;

        for (int i = 0; i < processes_[parent].children.size(); i++){
            if (processes_[parent].children[i] == currProcess){
                processes_[parent].children.erase(processes_[parent].children.begin() + i);
                break;
            }
        }

        readyQueue_.push_back(parent);
        processes_.erase(currProcess);
    }
    else{
        processes_.erase(currProcess);
    }

    // Empty the process from the queue and update the CPU
    if (!readyQueue_.empty()){
        int nextProcess = readyQueue_.front();
        readyQueue_.pop_front();
        cpu_ = nextProcess;
    }
    else{
        cpu_ = NO_PROCESS;
    }
} 

void SimOS::SimWait(){
    checkCpuState();
    int currProcess = cpu_;
    // No child no need to wait
    if (processes_[currProcess].children.empty()){
        return;
    }
    // Look for zombie child and remove them
    for (int i = 0; i < processes_[currProcess].children.size(); i++){
        int child = processes_[currProcess].children[i];
        if (processes_[child].isZombie){
            processes_.erase(child);
            processes_[currProcess].children.erase(processes_[currProcess].children.begin() + i);
            i--;
            return;
        }
    }
    processes_[currProcess].isWaiting = true;

    if (!readyQueue_.empty()){
        cpu_ = readyQueue_.front();
        readyQueue_.pop_front();
    }
}

void SimOS::TimerInterrupt(){
    checkCpuState();
    // If there is something in the Queue let it use the CPU
    if (!readyQueue_.empty()){
        int nextProcess = readyQueue_.front();
        readyQueue_.pop_front();
        readyQueue_.push_back(cpu_);
        cpu_ = nextProcess;
    }
    // Else do nothing , current process should still be holding the CPU
}

void SimOS::DiskReadRequest( int diskNumber, std::string fileName ){
    checkCpuState();
    checkDiskState(diskNumber);
    int currPID = cpu_;
    if (!readyQueue_.empty()){
        int nextProcess = readyQueue_.front();
        readyQueue_.pop_front();
        cpu_ = nextProcess;
    }
    else{
        cpu_ = NO_PROCESS;
    }

    FileReadRequest fileReadRequest;
    fileReadRequest.PID = currPID;
    fileReadRequest.fileName = fileName;

    Disk& currDisk = disks_[diskNumber];
    int diskState = currDisk.currServing.PID;

    // If the disk is not currently serving any files assign it
    if (!diskState){
        currDisk.currServing =  fileReadRequest;
    }
    // Else put it in the IO queue of the disk and wait for it's turn
    else{
        currDisk.diskQueue.push_back(fileReadRequest);
    }
}

void SimOS::DiskJobCompleted( int diskNumber ){
    checkDiskState(diskNumber);

    Disk& currDisk = disks_[diskNumber];
    int currPID = currDisk.currServing.PID;

    //If the disk is not serving any files do nothing and return
    if (!currPID){
        return;
    }

    //Clear the queue
    if (!currDisk.diskQueue.empty()){
        currDisk.currServing = currDisk.diskQueue.front();
        currDisk.diskQueue.pop_front();
    }
    else{
        currDisk.currServing = FileReadRequest{};
    }
    
    //Now manage the CPU part
    if(!cpu_){
        cpu_ = currPID;
    }
    else{
        readyQueue_.push_back(currPID);
    }
}

void SimOS::AccessMemoryAddress(unsigned long long address){
    checkCpuState();
    unsigned long long pageNumber = address / pageSize_;
    // Check if the page is already in memory or not
    for (const auto& frame: frames_){
        // if the page is already in memory then we can just put it in the back of the queue
        if (frame.pageNumber == pageNumber && frame.PID == cpu_){
            for (int i = 0; i < lruOrder_.size(); i++){
                int PID = lruOrder_[i].first;
                unsigned long long pageNum = lruOrder_[i].second;
                if ( PID == cpu_ && pageNum == pageNumber){
                    lruOrder_.erase(lruOrder_.begin() + i);
                    lruOrder_.push_back({PID, pageNum});
                    break;
                }
            }
            return;
        }
    }
    // Not in memory, so create and push it to the first avalible frame
    if (!emptyFrames_.empty()){
            unsigned long long emptyFrame = emptyFrames_[0];
            emptyFrames_.erase(emptyFrames_.begin());
            frames_.push_back({pageNumber, emptyFrame, cpu_});
            lruOrder_.push_back({cpu_, pageNumber});
            return;
    }
    int oldPID = lruOrder_.front().first;
    unsigned long long oldPage = lruOrder_.front().second;
    lruOrder_.pop_front();
    for (int i = 0; i < frames_.size(); i++){
        if (frames_[i].PID == oldPID && frames_[i].pageNumber == oldPage){
            frames_[i].PID = cpu_;
            frames_[i].pageNumber = pageNumber;
            break;
        }
    }
    lruOrder_.push_back({cpu_, pageNumber});
}

int SimOS::GetCPU(){
    return cpu_;
}

std::deque<int> SimOS::GetReadyQueue(){
    return readyQueue_;
}

MemoryUsage SimOS::GetMemory(){
    MemoryUsage memoryUsage = frames_;
    std::sort(memoryUsage.begin(), memoryUsage.end(), [](auto a, auto b){
        return a.frameNumber < b.frameNumber;
    });
    return memoryUsage;
}

FileReadRequest SimOS::GetDisk( int diskNumber ){
    checkDiskState(diskNumber);
    return disks_[diskNumber].currServing;
}

std::deque<FileReadRequest> SimOS::GetDiskQueue( int diskNumber ){
    checkDiskState(diskNumber);
    return disks_[diskNumber].diskQueue;
}
