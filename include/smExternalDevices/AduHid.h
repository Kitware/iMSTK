// AduHid.h - Controls I/O with Ontrak USB Hid devices - V1.0.0.0
//
// Copyright 2002 Ontrak Control Systems Incorporated
// By: John Homppi, Oct 28, 2002

#ifdef __CPLUSPLUS
extern "C"
{
#endif

    typedef struct _ADU_DEVICE_ID
    {
        unsigned short iVendorId;
        unsigned short iProductId;
        char    szSerialNumber[7];
    } ADU_DEVICE_ID, *PADU_DEVICE_ID;

// entry points for general use
    void __stdcall ShowAduDeviceList(ADU_DEVICE_ID* pAduDeviceId,
                                     char* psPrompt);


// entry points to support the ADU Device pipe
    void * __stdcall OpenAduDevice(unsigned long iTimeout);

    void * __stdcall OpenAduDeviceByProductId(int iProductId,
            unsigned long iTimeout);

    void * __stdcall OpenAduDeviceBySerialNumber(const char* psSerialNumber,
            unsigned long iTimeout);

    int __stdcall ReadAduDevice(void * hDevice,
                                void * lpBuffer,
                                unsigned long nNumberOfBytesToRead,
                                unsigned long * lpNumberOfBytesRead,
                                unsigned long iTimeout);

    int __stdcall WriteAduDevice(void * hDevice,
                                 const void * lpBuffer,
                                 unsigned long nNumberOfBytesToWrite,
                                 unsigned long * lpNumberOfBytesWritten,
                                 unsigned long iTimeout);

    void __stdcall CloseAduDevice(void * hDevice);


// entry points to support the RS232 pipe

    void * __stdcall OpenAdu232(unsigned long iTimeout);

    void * __stdcall OpenAdu232ByProductId(int iProductId,
                                           unsigned long iTimeout);

    void * __stdcall OpenAdu232BySerialNumber(const char* psSerialNumber,
            unsigned long iTimeout);

    int __stdcall ReadAdu232(void * hRS232,
                             void * lpBuffer,
                             unsigned long nNumberOfBytesToRead,
                             unsigned long * lpNumberOfBytesRead,
                             unsigned long iTimeout);

    int __stdcall WriteAdu232(void * hRS232,
                              const void * lpBuffer,
                              unsigned long nNumberOfBytesToWrite,
                              unsigned long * lpNumberOfBytesWritten,
                              unsigned long iTimeout);

    void __stdcall CloseAdu232(void * hRS232);


// entry points to support the Streaming pipes
    void * __stdcall OpenAduStream(unsigned long iTimeout);

    void * __stdcall OpenAduStreamByProductId(int iProductId,
            unsigned long iTimeout);

    void * __stdcall OpenAduStreamBySerialNumber(const char* psSerialNumber,
            unsigned long iTimeout);

    int __stdcall ReadAduStream(void * hStream,
                                void * lpBuffer,
                                unsigned long nNumberOfBytesToRead,
                                unsigned long * lpNumberOfBytesRead,
                                unsigned long iTimeout);

    void __stdcall CloseAduStream(void * hStream);

#ifdef __CPLUSPLUS
}
#endif


