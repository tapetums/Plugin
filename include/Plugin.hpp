// Plugin.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// プラグインオブジェクト
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>
#include <strsafe.h>

#include "Types.hpp"

//---------------------------------------------------------------------------//

interface IData;
typedef HRESULT (__stdcall* PluginFunc)(U8CSTR msg, IData* data);

//---------------------------------------------------------------------------//

class Plugin final
{
public:
    LPWSTR  path;
    HMODULE handle;
    IData*  (__stdcall* GetInfo)(size_t);
    IData*  (__stdcall* GetData)(size_t, U8CSTR);
    HRESULT (__stdcall* SetData)(size_t, IData*);
    HRESULT (__stdcall* Attach) (U8CSTR, PluginFunc);
    HRESULT (__stdcall* Detach) (U8CSTR, PluginFunc);
    HRESULT (__stdcall* Notify) (U8CSTR, IData*);

public:
    explicit Plugin(LPCWSTR plugin_path = nullptr)
    {
        path = new WCHAR[MAX_PATH];

        path[0] = '\0';
        handle  = nullptr;
        GetInfo = nullptr;
        GetData = nullptr;
        SetData = nullptr;
        Attach  = nullptr;
        Detach  = nullptr;
        Notify  = nullptr;

        if ( plugin_path )
        {
            this->Load(plugin_path);
        }
    }

    ~Plugin()
    {
        if ( handle )
        {
            this->Free();
        }
        if ( path )
        {
            delete path;
            path = nullptr;
        }
    }

    Plugin(Plugin&& rhs)
    {
        path    = rhs.path;
        handle  = rhs.handle;
        GetInfo = rhs.GetInfo;
        GetData = rhs.GetData;
        SetData = rhs.SetData;
        Attach  = rhs.Attach;
        Detach  = rhs.Detach;
        Notify  = rhs.Notify;

        rhs.path    = nullptr;
        rhs.handle  = nullptr;
        rhs.GetInfo = nullptr;
        rhs.GetData = nullptr;
        rhs.SetData = nullptr;
        rhs.Attach  = nullptr;
        rhs.Detach  = nullptr;
        rhs.Notify  = nullptr;
    }

    Plugin& operator ==(Plugin&& rhs)
    {
        path    = rhs.path;
        handle  = rhs.handle;
        GetInfo = rhs.GetInfo;
        GetData = rhs.GetData;
        SetData = rhs.SetData;
        Attach  = rhs.Attach;
        Detach  = rhs.Detach;
        Notify  = rhs.Notify;

        rhs.path    = nullptr;
        rhs.handle  = nullptr;
        rhs.GetInfo = nullptr;
        rhs.GetData = nullptr;
        rhs.SetData = nullptr;
        rhs.Attach  = nullptr;
        rhs.Detach  = nullptr;
        rhs.Notify  = nullptr;

        return *this;
    }

public:
    HRESULT __stdcall Load(LPCWSTR plugin_path)
    {
        if ( handle )
        {
            return S_FALSE;
        }

        handle = ::LoadLibraryW(plugin_path);
        if ( nullptr == handle )
        {
            return E_INVALIDARG;
        }

        ::StringCchCopyW(path, MAX_PATH, plugin_path);

        GetInfo = (decltype(GetInfo))::GetProcAddress(handle, "GetInfo");
        GetData = (decltype(GetData))::GetProcAddress(handle, "GetData");
        SetData = (decltype(SetData))::GetProcAddress(handle, "SetData");
        Attach  = (decltype(Attach)) ::GetProcAddress(handle, "Attach");
        Detach  = (decltype(Detach)) ::GetProcAddress(handle, "Detach");
        Notify  = (decltype(Notify)) ::GetProcAddress(handle, "Notify");

        if
        (
            GetInfo == nullptr ||
            GetData == nullptr ||
            SetData == nullptr ||
            Attach  == nullptr ||
            Detach  == nullptr ||
            Notify  == nullptr
        )
        {
            this->Free();
            return E_FAIL;
        }

        return S_OK;
    }

    HRESULT __stdcall Free()
    {
        HRESULT hr = S_OK;

        if ( handle )
        {
            ::FreeLibrary(handle);
        }
        else
        {
            hr = S_FALSE;
        }

        path[0] = '\0';
        handle  = nullptr;
        GetInfo = nullptr;
        GetData = nullptr;
        SetData = nullptr;
        Attach  = nullptr;
        Detach  = nullptr;
        Notify  = nullptr;

        return hr;
    }

private:
    Plugin(const Plugin& lhs)              = delete;
    Plugin& operator ==(const Plugin& lhs) = delete;
};

//---------------------------------------------------------------------------//

// Plugin.hpp