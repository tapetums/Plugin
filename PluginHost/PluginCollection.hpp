// PluginCollection.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// プラグインコレクションオブジェクト
//  内部のコンテナ構造を隠蔽するため
//  ICollectionインターフェイスを実装している
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <windows.h>

#include "Types.hpp"
#include "Collection.hpp"
#include "Plugin.hpp"

//---------------------------------------------------------------------------//

typedef ICollection<IUnknown, Plugin*, Plugin&&, U8CSTR> IPluginCollection;

//---------------------------------------------------------------------------//

class PluginCollection : public IPluginCollection
{
public:
    PluginCollection();
    ~PluginCollection();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    size_t  __stdcall size()           const override;
    Plugin* __stdcall at(size_t index) const override;

    HRESULT            __stdcall Append(Plugin&& plugin)   override;
    HRESULT            __stdcall Remove(Plugin&& plugin)   override;
    HRESULT            __stdcall Clear()                  override;
    IPluginCollection* __stdcall Collect(Condition cond)  override;
    Plugin*            __stdcall Find(U8CSTR plugin_name) override;

protected:
    ULONG m_cRef = 0;

private:
    struct Impl;
    Impl* pimpl;

private:
    PluginCollection(const PluginCollection&)             = delete;
    PluginCollection(PluginCollection&&)                  = delete;
    PluginCollection& operator =(const PluginCollection&) = delete;
    PluginCollection& operator =(PluginCollection&&)      = delete;
};

//---------------------------------------------------------------------------//

HRESULT __stdcall SearchPlugins
(
    LPCWSTR dir_path, PluginCollection& collection, bool search_in_sub
);

//---------------------------------------------------------------------------//

// PluginCollection.hpp