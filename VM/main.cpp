#include <iostream>
#include <fstream>
#include <sstream>
#include "Shell.hpp"
#include "VM.hpp"

int main(int argc, char *argv[])
{
    std::cout << "Starting B1rd-V1rtual-Mach1ne";
    if (argc < 2)
    {
        Shell();
    }
    else
    {
        AddProgram(argv[1]);
        Shell();
    }
    std::cout << "Closing B1rd-V1rtual-Mach1ne";
    return 0;
}