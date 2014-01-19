#ifndef NAMEDPIPE_H_
#define NAMEDPIPE_H_
#include <cstdio>
class  NamedPipe{
    private :
        const static int MAX_RETRY = 5;
        const static int BUFFER_SIZE = 100;
        char fifo_path[BUFFER_SIZE];
        int fifo; // file descriptor

    public :
        NamedPipe(const char* fifo_path);
        ~NamedPipe();
        bool CreatePipe();
        bool Exist();
        int ReadFromPipe(char* buffer, int buf_size);   // Return value : On success, buffer is returned. On failure, NULL is returned.
        int WriteToPipe(const void* data, int size); // Return value : On success, the number of bytes written. On failure, negative value.
        bool ensure_open(char mode);
        void closePipe();
};
#endif
