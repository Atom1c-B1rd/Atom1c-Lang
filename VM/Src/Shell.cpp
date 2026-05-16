#include "Shell.hpp"
#include "VM.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>


void Help()
{
    std::cout << "======Help======" << std::endl;
}
void Shell()
{
    bool running = true;
    std::string Input;
    std::cout << "Wellcome to B1rd Shell Print Help for see all Commands" << std::endl;
    while (running)
    {
        std::cout << "-->";
        std::getline(std::cin, Input);
        size_t start = Input.find_first_not_of(" \t");
        if (start == std::string::npos)
        {
            continue;
        }
        Input = Input.substr(start);

        std::string cmd_lower = Input;
        std::transform(cmd_lower.begin(), cmd_lower.end(), cmd_lower.begin(),
                       [](unsigned char c)
                       { return std::tolower(c); });

        std::istringstream iss(cmd_lower);
        std::string command;
        iss >> command;
        if (command != "exit")
        {
            if (command == "help")
            {
                Help();
            }
            else if(command == "add"){
                size_t pos= Input.find_first_of(" \t");
                if(pos!=std::string::npos){
                    std::string arg =Input.substr(pos+1);
                    AddProgram(arg.c_str());
                }
            }else if(command == "run"){
                size_t pos= Input.find_first_of(" \t");
                if(pos!=std::string::npos){
                    std::string arg =Input.substr(pos+1);
                    RunProgram(arg.c_str());
                }
            }
            else
            {
                std::cout << "UnKnown Command: " + Input + " " << std::endl;
            }
        }
        else
        {
            break;
        }
    }
}