#ifndef _NET2COM_H_
#define _NET2COM_H_
#include <NamedPipe.h>

typedef enum {
    Dnet_w_com_r = 0,
    Dcom_w_net_r = 1,
    Inet_w_com_r = 2,
    Icom_w_net_r = 3
} pipe_num_t;

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
        Net2Com(pipe_num_t dataw, pipe_num_t datar, pipe_num_t infow, pipe_num_t infor);        // ORDER : DATA_WRITE, DATA_READ, INFO_WRITE, INFO_READ
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
