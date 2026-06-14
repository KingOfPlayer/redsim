#include "freefem.h"

FreeFemModule::FreeFemModule() {}

FreeFemModule::~FreeFemModule() {
    AbortSimulation(); 
    if (asyncWorker.valid()) {
        asyncWorker.wait();
    }
}

bool FreeFemModule::runSimulationTask(const std::string& scriptPath) {
    currentStatus = FreeFemStatus::Running;

    // Check if script file exists
    if (access(scriptPath.c_str(), F_OK) == -1) {
        std::lock_guard<std::mutex> lock(logMutex);
        outputLog = "Error: FreeFEM script file not found at path: " + scriptPath;
        currentStatus = FreeFemStatus::Failed;
        return false;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::lock_guard<std::mutex> lock(logMutex);
        outputLog = "Error: Failed to create pipe for inter-process communication!";
        currentStatus = FreeFemStatus::Failed;
        return false;
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::lock_guard<std::mutex> lock(logMutex);
        outputLog = "Error: Failed to fork process for FreeFEM execution!";
        currentStatus = FreeFemStatus::Failed;
        close(pipefd[0]);
        close(pipefd[1]);
        return false;
    }

    if (pid == 0) {
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        close(pipefd[0]);
        close(pipefd[1]);

        execlp("ff-mpirun", "ff-mpirun", "-np", "1", scriptPath.c_str(), "-wg", nullptr); 

        printf("Error: Failed to execute FreeFEM script!\n");
        _exit(1);
    } else {
        childPid.store(pid);
        close(pipefd[1]);

        char buffer[512];
        ssize_t bytesRead;
        
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            std::lock_guard<std::mutex> lock(logMutex);
            outputLog += buffer;
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        childPid.store(-1);

        if (currentStatus.load() == FreeFemStatus::Aborted) {
            return false;
        }

        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            currentStatus = FreeFemStatus::Success;
            return true;
        } else {
            currentStatus = FreeFemStatus::Failed;
            return false;
        }
    }
}

void FreeFemModule::StartSimulation(const std::string& scriptPath) {
    if (currentStatus == FreeFemStatus::Running) {
        return;
    }
    
    {
        std::lock_guard<std::mutex> lock(logMutex);
        outputLog.clear();
    }

    asyncWorker = std::async(std::launch::async, &FreeFemModule::runSimulationTask, this, scriptPath);
}

void FreeFemModule::AbortSimulation() {
    pid_t pid = childPid.load();
    if (pid > 0) {
        currentStatus = FreeFemStatus::Aborted;
        kill(pid, SIGINT); 
        
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (childPid.load() == pid) {
            kill(pid, SIGKILL); 
        }
        
        std::lock_guard<std::mutex> lock(logMutex);
        outputLog += "\n[FreeFemModule] Stop signal sent to FreeFEM process.";
    }
}

FreeFemStatus FreeFemModule::GetStatus() const { 
    return currentStatus.load(); 
}

bool FreeFemModule::IsFinished() const {
    FreeFemStatus s = currentStatus.load();
    return (s == FreeFemStatus::Success || s == FreeFemStatus::Failed || s == FreeFemStatus::Aborted);
}

std::string FreeFemModule::GetOutputLog() {
    std::lock_guard<std::mutex> lock(logMutex);
    return outputLog;
}