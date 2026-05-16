#ifndef KERNEL_HPP 
#define KERNEL_HPP
#include "Process.hpp"
enum class KernelSignals{
    //SystemLoop
    KernelOn,
    KernelOff,
    //
    KernelError,
    KernelInterrupt,
    //
    KernelBackUp,
};
typedef struct{
    KernelSignals Signal;
    Process procs;
}KernelSignal;

KernelSignal EmitSignalKernel();

#endif