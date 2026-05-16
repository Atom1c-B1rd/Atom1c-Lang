#ifndef TASK_HPP
#define TASK_HPP
#include "Indications.hpp"
#include <vector>
enum class TaskSignals{
    StartTask,
    InterruptTask,
    RunningTask,
    EndTask,
};

typedef struct
{
    std::vector<Indication> Indications;
    size_t ip;
}Task;

typedef struct{
    Task task;
    TaskSignals Signal;
    std::string text;
}TaskSignal;


Task MakeTask(Indication indication);
TaskSignal EmitSignalTask();

#endif