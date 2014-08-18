// ConsoleOut.cpp

//---------------------------------------------------------------------------//
//
// デバッグウィンドウへの出力関数
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#if defined(_DEBUG) || defined(DEBUG)

#include <windows.h>
#include <strsafe.h>

#include "ConsoleOut.hpp"

//---------------------------------------------------------------------------//

#define CONSOLE_BUFSIZE 1024
#define thread_local __declspec(thread) static

//---------------------------------------------------------------------------//

void __stdcall console_outA(const char* format, ...)
{
    thread_local char buf[CONSOLE_BUFSIZE];

    // 引数を文字列に
    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfA(buf, CONSOLE_BUFSIZE, format, al);
    }
    va_end(al);

    ::OutputDebugStringA(buf);
    ::OutputDebugStringA("\n");
}

//---------------------------------------------------------------------------//

void __stdcall console_outW(const wchar_t* format, ...)
{
    thread_local wchar_t buf[CONSOLE_BUFSIZE];

    // 引数を文字列に
    va_list al;
    va_start(al, format);
    {
        ::StringCchVPrintfW(buf, CONSOLE_BUFSIZE, format, al);
    }
    va_end(al);

    ::OutputDebugStringW(buf);
    ::OutputDebugStringW(L"\n");
}

//---------------------------------------------------------------------------//

#endif // #if defined(_DEBUG) || defined(DEBUG)

//---------------------------------------------------------------------------//

// ConsoleOut.cpp