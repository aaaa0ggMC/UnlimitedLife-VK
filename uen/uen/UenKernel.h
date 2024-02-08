#ifndef UENKERNEL_H_INCLUDED
#define UENKERNEL_H_INCLUDED

#ifndef UENEX
#ifdef BUILD_DLL
    #define UENEX __declspec(dllexport)
#else
    #define UENEX __declspec(dllimport)
#endif
#endif // UENEX

#endif // UENKERNEL_H_INCLUDED
