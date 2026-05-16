#ifndef VM_HPP
#define VM_HPP
#include "Decoder.hpp"
#include "Kernel.hpp"
#include "Process.hpp"
#include <vector>

enum class VMIndications{
    //System
    PowerOn,
    PowerOff,
    //Kernel
    KernelCall,
    KernelResponse,
    KernelInterrupt,
    //Process
    ProcessCall,
    ProcessResponse,
    ProcessInterrupt,
};
typedef struct{
    std::string Name;
    KernelSignal kernelHelp;
    std::vector<Process> Prog;
    size_t ProgIndex;
}Program;


void PowerOn();
void PowerOff();

void KernelLoop();
void ProcessLoop();

void mainLoop();
Program makeProgram(std::string source);
void AddProgram(const char* path);
void RunProgram(const char *name);

#endif