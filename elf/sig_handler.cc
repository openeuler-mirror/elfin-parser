#include "sig_handler.hh"

void sigabrt_handler(int signal_num)
{
        std::cout << "Error : Receive the signal SIGABRT!" << std::endl;
        std::cout << "Please double check if the ELF file has been maliciously modified." << std::endl;
        exit(1);
}

void sigsegv_handler(int signal_num)
{
        std::cout << "Error : Receive the signal SIGSEGV!" << std::endl;
        std::cout << "Please double check if the ELF file has been maliciously modified." << std::endl;
        exit(1);
}