#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ratio>

#include <iostream>
#include <chrono>

const unsigned long long size = 500000000;
char buffer[size];

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        std::cout << "Need set file name" << std::endl;
        return 1;
    }
    //char a;

    std::cout << "First parameters is: " << argv[1] 
              << " Total bytes copy is: " << size << std::endl;
    int pFile, pFileRead;
    pFileRead = ::open("/dev/urandom", O_RDONLY, S_IREAD);
    std::cout << pFileRead << std::endl;




    pFile = open(argv[1], O_WRONLY, S_IWRITE);
    std::cout << pFile << std::endl;
    for (unsigned long long j = 0; j < 5; ++j)
    {
        //auto t1 = std::chrono::high_resolution_clock::now();
/*        ::read(pFileRead, buffer, size / 2);
        std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count() << std::endl;
        t1 = std::chrono::high_resolution_clock::now();*/
        ::write(pFile, buffer, size / 5);
        //std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count() << std::endl;

        //Some calculations to fill a[]
        //fwrite(a, 1, size*sizeof(unsigned long long), pFile);
    }

        syncfs(pFile);

    fcloseall();


    return 0;
}