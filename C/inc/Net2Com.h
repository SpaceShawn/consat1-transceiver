#ifndef _NET2COM_H_
#define _NET2COM_H_
#include <NamedPipe.h>

enum PIPE_NUM{
    PIPE_ONE, PIPE_TWO, PIPE_THREE, PIPE_FOUR
};

class Net2Com{
    private :
        const static int NULL_CHAR_LENGTH = 1;
        static const int NUMBER_OF_PIPES = 4;
        static const char* pipe_str[];
         
        NamedPipe* pipe[NUMBER_OF_PIPES];
        
        NamedPipe* dataPipe_w;
        NamedPipe* dataPipe_r;
        NamedPipe* infoPipe_w;
        NamedPipe* infoPipe_r;

    public :
        Net2Com(PIPE_NUM dataw, PIPE_NUM datar, PIPE_NUM infow, PIPE_NUM infor);        // ORDER : DATA_WRITE, DATA_READ, INFO_WRITE, INFO_READ
        ~Net2Com();
        int WriteToDataPipe(const char* str);
        int WriteToDataPipe(unsigned char);
        int WriteToDataPipe(const void*, int);
        int ReadFromDataPipe(char* buffer, int buf_size);
        int WriteToInfoPipe(const char* str);
        int WriteToInfoPipe(const void*, int);
        int WriteToInfoPipe(unsigned char);
        int ReadFromInfoPipe(char* buffer, int buf_size);

        void OpenReadPipesPersistently();                                               // If you are using this mode, you have to 
        void OpenWritePipesPersistently();                                              // persistently open BOTH sides, otherwise it blocks.

    private :
        bool Initialize();
        bool CreatePipes();
};
#endif
