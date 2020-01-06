#ifndef __STANDARTMACROS_H
#define __STANDARTMACROS_H


#define DDSD_STANDARTINIT(a) ZeroMemory(&a,sizeof(a));a.dwSize=sizeof(a)

#ifdef __DLL__
#define DLLCLASS __declspec(dllexport) // Wenn DLL Compiliert dann exportierbar
#else  
  #ifdef __IMPORTDLL__
  #define DLLCLASS __declspec(dllimport) // Wenn DLL importiert wird: importierbar
  #else 
  #define  DLLCLASS      // Wenn Klassen statisch gelinkt werden sollen
  #endif
#endif


#endif