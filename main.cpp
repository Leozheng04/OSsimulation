#include "SimOS.h"
#include <iostream>

int main(){
    SimOS os(2, 1024, 16);
    os.NewProcess(); // 1
    os.NewProcess(); // 2
    os.NewProcess(); // 3
    // os.NewProcess();
    // os.SimExit();
    // os.SimExit();
    // os.SimExit();

    os.TimerInterrupt(); // 2
    os.TimerInterrupt(); // 3
    // os.TimerInterrupt(); // 1
    std::cout << os.GetCPU();

    std::cout << std::endl;

    std::deque<int> q = os.GetReadyQueue();

    for (int pid : q){
        std::cout << pid << " ";
    }
    std::cout << std::endl;

    
    return 0;
}