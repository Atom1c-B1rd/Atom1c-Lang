#ifndef PROCESS_HPP
#define PROCESS_HPP
#include "Task.hpp"
enum class ProcessSignals{
    Process_Start,
    Process_Interrupt,
    Process_Runnign,
    Process_End,
};
typedef struct{
    std::vector<Task> Tasks;
    size_t currentTask;
    std::vector<int> stack;
    bool running;
}Process;
typedef struct{
    Process process;
    ProcessSignals signal;
    std::string text;
}ProcessSignal;

Process MakeProcess(Task tarea);
ProcessSignal EmitProcessSignal();
#endif