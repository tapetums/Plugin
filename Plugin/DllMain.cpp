// DllMain.cpp

#pragma execution_character_set("utf-8") 

//---------------------------------------------------------------------------//

#include <string>
#include <unordered_map>

#include <windows.h>

#include "ConsoleOut.hpp"
#include "Types.hpp"
#include "SRWLock.hpp"
#include "Data.hpp"
#include "Plugin.hpp"

#define thread_local __declspec(thread) static

#define NAME TEXT("Test")

//---------------------------------------------------------------------------//

namespace std { typedef basic_string<char8_t> u8string; }

typedef std::unordered_multimap<std::u8string, PluginFunc> MessageMap;

//---------------------------------------------------------------------------//

static SRWLock::LockHolder lock_holder;
static MessageMap msg_map;

static Data<U8CSTR> info_00((U8CSTR)"name",        (U8CSTR)"Test");
static Data<U8CSTR> info_01((U8CSTR)"description", (U8CSTR)"Test plugin");
static Data<U8CSTR> info_02((U8CSTR)"copyright",   (U8CSTR)"Copyright (C) 2014 tapetums");

static IData* info[] =
{
    &info_00, &info_01, &info_02,
};

//---------------------------------------------------------------------------//

BOOL __stdcall DllMain(HINSTANCE, DWORD, LPVOID)
{
    return TRUE;
}

//---------------------------------------------------------------------------//

extern "C" IData* __stdcall GetInfo(size_t index)
{
    static const auto info_count = sizeof(info) / sizeof(IData*);

    if ( index < info_count )
    {
        return info[index];
    }
    else
    {
        return nullptr;
    }
}

//---------------------------------------------------------------------------//

extern "C" IData* __stdcall GetData(size_t index, U8CSTR type_as)
{
    return nullptr;
}

//---------------------------------------------------------------------------//

extern "C" HRESULT __stdcall SetData(size_t index, IData* data)
{
    return E_NOTIMPL;
}

//---------------------------------------------------------------------------//

extern "C" HRESULT __stdcall Attach(U8CSTR msg, PluginFunc listener)
{
    console_out(TEXT("%s::Attach() begin"), NAME);

    if ( nullptr == listener )
    {
        console_out(TEXT("listener is null"));
        console_out(TEXT("%s::Attach() end"), NAME);
        return E_POINTER;
    }

    // デバッグ情報の表示
    console_outA("msg: %s", (char*)msg);
    console_outA("listener: 0x%x", listener);

    // 文字列比較のため msg を std::u8string に変換
    const auto msg_string = std::u8string(msg);

    // 重複チェック
    {
        SRWLock::ReadLock lock(lock_holder);

        const auto range = msg_map.equal_range(std::move(msg_string));
        const auto end = range.second;
        for ( auto it = range.first; it != end; ++it )
        {
            if ( it->second == listener )
            {
                console_out(TEXT("Already attached"));
                console_out(TEXT("%s::Attach() end"), NAME);
                return S_FALSE;
            }
        }
    }

    // メッセージマップに登録
    {
        SRWLock::WriteLock lock(lock_holder);

        msg_map.emplace(msg, listener);
    }

    console_out(TEXT("Attachehed"));
    console_out(TEXT("%s::Attach() end"), NAME);

    return S_OK;
}

//---------------------------------------------------------------------------//

extern "C" HRESULT __stdcall Detach(U8CSTR msg, PluginFunc listener)
{
    console_out(TEXT("%s::Detach() begin"), NAME);

    if ( nullptr == listener )
    {
        console_out(TEXT("listener is null"));
        console_out(TEXT("%s::Detach() end"), NAME);
        return E_POINTER;
    }

    // デバッグ情報の表示
    console_outA("msg: %s", (char*)msg);
    console_outA("listener: 0x%x", listener);

    // 文字列比較のため msg を std::u8string に変換
    const auto msg_string = std::u8string(msg);

    HRESULT hr = S_FALSE;

    // 解除対象をさがす
    {
        SRWLock::WriteLock lock(lock_holder);

        const auto range = msg_map.equal_range(std::move(msg_string));
        const auto end = range.second;
        for ( auto it = range.first; it != end; ++it )
        {
            if ( it->second == listener )
            {
                // メッセージマップから登録解除
                msg_map.erase(it);

                console_out(TEXT("Detachehed"));
                console_out(TEXT("%s::Detach() end"), NAME);
                return S_OK;
            }
        }
    }

    console_out(TEXT("Not been attached"));
    console_out(TEXT("%s::Detach() end"), NAME);

    return hr;
}

//---------------------------------------------------------------------------//

extern "C" HRESULT __stdcall Notify(U8CSTR msg, IData* data)
{
    console_out(TEXT("%s::Notify() begin"), NAME);

    thread_local size_t count_handled = 0;

    // デバッグ情報の表示
    console_outA("msg: %s", (char*)msg);
    console_outA("data: 0x%x", data);

    // 文字列比較のため msg を std::u8string に変換
    const auto msg_string = std::u8string(msg);

    // ...

    // メッセージチェーンに処理を投げる
    {
        SRWLock::ReadLock lock(lock_holder);

        const auto range = msg_map.equal_range(std::move(msg_string));
        const auto end = range.second;
        for ( auto it = range.first; it != end; ++it )
        {
            const auto listener = it->second;
            if ( nullptr == listener )
            {
                continue;
            }

            const auto hr = listener(msg, data);
            if ( SUCCEEDED(hr) )
            {
                ++count_handled;
            }
        }
    }

    // ...

    if ( 0 == count_handled )
    {
        console_out(TEXT("The message was handled by no listener"));
    }

    console_out(TEXT("%s::Notify() end"), NAME);

    return count_handled ? S_OK : S_FALSE;
}

//---------------------------------------------------------------------------//

// DllMain.cpp