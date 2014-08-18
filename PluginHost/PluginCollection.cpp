// PluginCollection.cpp

#pragma execution_character_set("utf-8") 

//---------------------------------------------------------------------------//
//
// プラグインコレクションオブジェクト
//  内部のコンテナ構造を隠蔽するため
//  ICollectionインターフェイスを実装している
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <vector>

#include <windows.h>
#include <strsafe.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include "ConsoleOut.hpp"
#include "SRWLock.hpp"
#include "ComPtr.hpp"
#include "Data.hpp"

#include "PluginCollection.hpp"

//---------------------------------------------------------------------------//
//
// Pimpl イディオム
//
//---------------------------------------------------------------------------//

struct PluginCollection::Impl
{
    SRWLock::LockHolder lock_holder;
    std::vector<Plugin> plugins;
};

//---------------------------------------------------------------------------//
//
// ctor / dtor
//
//---------------------------------------------------------------------------//

PluginCollection::PluginCollection()
{
    console_out(TEXT("%s::ctor() begin"), TEXT(__FILE__));

    this->AddRef();

    pimpl = new Impl;

    console_out(TEXT("%s::ctor() end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//

PluginCollection::~PluginCollection()
{
    console_out(TEXT("%s::dtor() begin"), TEXT(__FILE__));

    delete pimpl;
    pimpl = nullptr;

    console_out(TEXT("%s::dtor() end"), TEXT(__FILE__));
}

//---------------------------------------------------------------------------//
//
// IUnknownメンバ関数
//
//---------------------------------------------------------------------------//

HRESULT __stdcall PluginCollection::QueryInterface
(
    REFIID riid, void** ppvObject
)
{
    console_out(TEXT("%s::QueryInterface() begin"), TEXT(__FILE__));

    if ( nullptr == ppvObject )
    {
        return E_INVALIDARG;
    }

    *ppvObject = nullptr;

    if ( IsEqualIID(riid, IID_IUnknown) )
    {
        console_out(TEXT("IID_IUnknown"));
        *ppvObject = static_cast<IUnknown*>(this);
    }
    else if ( IsEqualIID(riid, IID_ICollection) )
    {
        console_out(TEXT("IID_ICollection"));
        *ppvObject = static_cast<ICollection*>(this);
    }
    else
    {
        console_out(TEXT("No such an interface"));
        console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));
        return E_NOINTERFACE;
    }

    this->AddRef();

    console_out(TEXT("%s::QueryInterface() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

ULONG __stdcall PluginCollection::AddRef()
{
    const auto cRef = ::InterlockedIncrement(&m_cRef);

    console_out(TEXT("%s::AddRef() %d -> %d"), TEXT(__FILE__), cRef - 1, cRef);

    return cRef;
}

//---------------------------------------------------------------------------//

ULONG __stdcall PluginCollection::Release()
{
    const auto cRef = ::InterlockedDecrement(&m_cRef);

    console_out(TEXT("%s::Release() %d -> %d"), TEXT(__FILE__), cRef + 1, cRef);

    if ( cRef == 0 )
    {
        m_cRef = ULONG_MAX;

        console_out(TEXT("%s::delete begin"), TEXT(__FILE__));
        {
            delete this;
        }
        console_out(TEXT("%s::delete end"), TEXT(__FILE__));
    }

    return cRef;
}

//---------------------------------------------------------------------------//

size_t __stdcall PluginCollection::size() const
{
    return pimpl->plugins.size();
}

//---------------------------------------------------------------------------//
//
// プロパティ
//
//---------------------------------------------------------------------------//

Plugin* __stdcall PluginCollection::at(size_t index) const
{
    Plugin* p_plugin = nullptr;

    try
    {
        SRWLock::ReadLock lock(pimpl->lock_holder);
        p_plugin = &pimpl->plugins.at(index);
    }
    catch ( std::out_of_range& )
    {
        p_plugin = nullptr;
    }

    return p_plugin;
}

//---------------------------------------------------------------------------//
//
// メンバ関数
//
//---------------------------------------------------------------------------//

HRESULT __stdcall PluginCollection::Append
(
    Plugin&& plugin
)
{
    console_out(TEXT("%s::Append() begin"), TEXT(__FILE__));

    const auto handle = plugin.handle;
    if ( nullptr == handle )
    {
        console_out(TEXT("Plugin handle is null"));
        console_out(TEXT("%s::Append() end"), TEXT(__FILE__));
        return E_POINTER;
    }

    // データベースに登録
    {
        SRWLock::WriteLock lock(pimpl->lock_holder);
        pimpl->plugins.emplace_back(std::move(plugin));
    }

    console_out(TEXT("Appended"));
    console_out(TEXT("%s::Append() end"), TEXT(__FILE__));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginCollection::Remove
(
    Plugin&& plugin
)
{
    console_out(TEXT("%s::Remove() begin"), TEXT(__FILE__));

    const auto handle = plugin.handle;
    if ( nullptr == handle )
    {
        console_out(TEXT("Plugin handle is null"));
        console_out(TEXT("%s::Remove() end"), TEXT(__FILE__));
        return E_POINTER;
    }

    // データベースから削除
    {
        SRWLock::WriteLock lock(pimpl->lock_holder);

        const auto end = pimpl->plugins.end();
        for ( auto it = pimpl->plugins.begin(); it != end; ++it )
        {
            if ( it->handle == handle )
            {
                pimpl->plugins.erase(it);
                console_out(TEXT("Removed"));
                console_out(TEXT("%s::Remove() end"), TEXT(__FILE__));
                return S_OK;
            }
        }
    }

    console_out(TEXT("Not found"));
    console_out(TEXT("%s::Remove() end"), TEXT(__FILE__));

    return S_FALSE;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall PluginCollection::Clear()
{
    const auto hr = pimpl->plugins.size() ? S_OK : S_FALSE;

    pimpl->plugins.clear();

    return hr;
}

//---------------------------------------------------------------------------//

IPluginCollection* __stdcall PluginCollection::Collect
(
    Condition cond
)
{
    console_out(TEXT("%s::Collect() begin"), TEXT(__FILE__));

    if ( nullptr == cond )
    {
        console_out(TEXT("Invalid function pointer"));
        console_out(TEXT("%s::Collect() end"), TEXT(__FILE__));
        return nullptr;
    }

    const auto collection = new PluginCollection;

    // 条件に合致するプラグインを収集
    {
        SRWLock::ReadLock lock(pimpl->lock_holder);

        for ( auto& plugin: pimpl->plugins )
        {
            if ( cond(&plugin) )
            {
                collection->pimpl->plugins.emplace_back(plugin.path);
            }
        }
    }

    console_out(TEXT("%s::Collect() end"), TEXT(__FILE__));

    return collection;
}

//---------------------------------------------------------------------------//

Plugin* __stdcall PluginCollection::Find
(
    U8CSTR plugin_name
)
{
    Plugin* p_plugin = nullptr;

    console_out(TEXT("%s::Collect() begin"), TEXT(__FILE__));

    // 条件に合致するプラグインを捜す
    {
        SRWLock::ReadLock lock(pimpl->lock_holder);

        for ( auto& plugin: pimpl->plugins )
        {
            for ( size_t index = 0; ; ++index )
            {
                const ComPtr<IData> info = plugin.GetInfo(index);
                if ( nullptr == info )
                {
                    break;
                }

                const auto item_name = (const char*)info->name();
                if ( 0 != lstrcmpA(item_name, "name") )
                {
                    continue;
                }

                const auto item_data = (const char*)info->get();
                if ( 0 == lstrcmpA(item_data, (char*)plugin_name) )
                {
                    p_plugin = &plugin;
                    break;
                }
            }
        }
    }

    console_out(TEXT("%s::Collect() end"), TEXT(__FILE__));

    return p_plugin;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall LoadPlugin
(
    LPCWSTR plugin_path, PluginCollection& collection
)
{
    console_outW(plugin_path);

    Plugin plugin(plugin_path);
    if ( nullptr == plugin.handle )
    {
        return E_POINTER;
    }

    for ( size_t index = 0; ; ++index )
    {
        const ComPtr<IData> info = plugin.GetInfo(index);
        if ( nullptr == info )
        {
            break;
        }

        if ( 0 == lstrcmpA("U8CSTR", (const char*)info->type()) )
        {
            console_outA(R"(%s: "%s")", info->name(), info->get());
        }
    }

    collection.Append(std::move(plugin));

    return S_OK;
}

//---------------------------------------------------------------------------//

HRESULT __stdcall SearchPlugins
(
    LPCWSTR dir_path, PluginCollection& collection, bool search_in_sub
)
{
    console_outW(dir_path);

    WCHAR path[MAX_PATH] = { 0 };
    ::StringCchPrintfW(path, MAX_PATH, LR"(%s\*)", dir_path);

    WIN32_FIND_DATA fd = { 0 };
    const auto hFind = ::FindFirstFileW(path, &fd);
    if ( INVALID_HANDLE_VALUE == hFind )
    {
        return S_FALSE;
    }

    do
    {
        const auto filename = fd.cFileName;
        console_outW(filename);

        if ( filename && filename[0] == L'.' )
        {
            continue; // ファイル名がピリオドで始まっているものは飛ばす
        }
        else if ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )
        {
            continue; // 隠し属性を持つものは飛ばす
        }

        // フルパスを合成
        ::StringCchPrintfW(path, MAX_PATH, LR"(%s\%s)", dir_path, filename);

        if ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        {
            // フォルダだったら
            if ( search_in_sub )
            {
                SearchPlugins(path, collection, search_in_sub);
            }
        }
        else
        {
            // DLLファイルだったら
            if ( 0 == lstrcmp(::PathFindExtensionW(path), L".dll") )
            {
                LoadPlugin(path, collection);
            }
        }
    }
    while ( ::FindNextFileW(hFind, &fd) );

    ::FindClose(hFind);

    return S_OK;
}

//---------------------------------------------------------------------------//

// PluginCollection.cpp