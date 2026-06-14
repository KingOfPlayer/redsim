#pragma once
#include <future>
#include <atomic>
#include <mutex>
#include <string>
#include <memory>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <thread>
#include "freefemtype.h"
#include "freefemscript.h"

class FreeFemModule {
private:
    double EValue = 210e9;
    double PoissonRatioValue = 0.3;
    std::string outputLog;

    std::atomic<FreeFemStatus> currentStatus{FreeFemStatus::Idle};
    std::atomic<pid_t> childPid{-1};
    std::future<bool> asyncWorker;
    std::mutex logMutex;

    bool runSimulationTask(const std::string& scriptPath);

public:
    FreeFemModule();
    ~FreeFemModule();

    void StartSimulation(const std::string& scriptPath);
    void AbortSimulation();
    
    FreeFemStatus GetStatus() const;
    bool IsFinished() const;
    std::string GetOutputLog();
};