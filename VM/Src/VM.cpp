#include "VM.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cctype>

std::vector<Program> Programs;

void AddProgram(const char *path)
{
    std::cout << "Program Adding..." << std::endl;
    std::cout << "Reading File..." << std::endl;
    std::fstream archive(path);
    if (archive.is_open())
    {
        std::stringstream buffer;
        buffer << archive.rdbuf();
        std::string content = buffer.str();
        Program prog=makeProgram(content);
        if(prog.Name.c_str()==nullptr)
            return;
        Programs.push_back(prog);
        archive.close();
    }
    else
    {
        std::cerr << "Error to open File" << std::endl;
    }
}
Program makeProgram(std::string source){
    std::vector<Decoder> Decode_source= Decode(source);
    if(Decode_source.empty())
        {
        std::cout<<"Error: Make Decode"<<std::endl;
        return;
        }
    std::vector<Indication> indications;
    for(size_t i=0;i<Decode_source.size();i++){
        Indication indication=makeIndication(Decode_source[i]);
        indications.push_back(indication);
    }
    if(indications.empty())
        {
        std::cout<<"Error: Make Indications"<<std::endl;
        return;
    }
    std::vector<Task> Tasks;
    for(size_t i=0;i<indications.size();i++){
        Task task=MakeTask(indications[i]);
        Tasks.push_back(task);
    }
    if(Tasks.empty())
    {
        std::cout<<"Error: Make Tasks"<<std::endl;
        return;
    }
        
    std::vector<Process> Processes;
    for(size_t i=0;i<Tasks.size();i++){
        Process Proc=MakeProcess(Tasks[i]);
        Processes.push_back(Proc);
    }
    if(Processes.empty()){
        std::cout<<"Error: Make Process"<<std::endl;
        return;
    }
    Program program;
    program.Prog=Processes;
    program.ProgIndex-Processes.size();
    return program;
}

void RunProgram(const char *name)
{
    
}
void mainLoop()
{
    std::cout << "hello world" << std::endl;
}