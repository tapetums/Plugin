// Data.hpp

#pragma once

//---------------------------------------------------------------------------//
//
// データを保持するためのオブジェクト
//   Copyright (C) 2014 tapetums
//
//---------------------------------------------------------------------------//

#include <stdint.h>
#include <windows.h>
#include <strsafe.h>

//---------------------------------------------------------------------------//

#include "ConsoleOut.hpp"
#include "Types.hpp"

//---------------------------------------------------------------------------//

static const IID IID_IData =
{ 0x4fe37727, 0x3644, 0x43bf, { 0x9f, 0xea, 0x3e, 0xd2, 0x48, 0x35, 0x3d, 0xc5 } };

//---------------------------------------------------------------------------//

interface IData : public IUnknown
{
    virtual U8CSTR    __stdcall name() const = 0;
    virtual size_t    __stdcall size() const = 0;
    virtual uint32_t  __stdcall flag() const = 0;
    virtual U8CSTR    __stdcall type() const = 0;
    virtual uintptr_t __stdcall get()  const = 0;
    virtual HRESULT   __stdcall set(const uintptr_t value) = 0;
};

//---------------------------------------------------------------------------//

class CData : public IData
{
private:
    static const size_t MAX_DATA_NAME_LENGTH = 64;

public:
    explicit CData(U8CSTR name = nullptr)
    {
        if ( name )
        {
            ::StringCchCopyA((char*)m_name, MAX_DATA_NAME_LENGTH, (const char*)name);
        }
        else
        {
            ::StringCchCopyA((char*)m_name, MAX_DATA_NAME_LENGTH, "(noname)");
        }

        this->AddRef();
    }

    virtual ~CData()
    {
    }

public:
    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override
    {
        if ( nullptr == ppvObject )
        {
            return E_POINTER;
        }

        *ppvObject = nullptr;

        if ( IsEqualIID(riid, IID_IUnknown) )
        {
            *ppvObject = static_cast<IUnknown*>(this);
        }
        else if ( IsEqualIID(riid, IID_IData) )
        {
            *ppvObject = static_cast<IData*>(this);
        }
        else
        {
            return E_NOINTERFACE;
        }

        this->AddRef();

        return S_OK;
    }

    ULONG __stdcall AddRef() override
    {
        const auto cRef = ::InterlockedIncrement(&m_cRef);

        return cRef;
    }

    ULONG __stdcall Release() override
    {
        const auto cRef = ::InterlockedDecrement(&m_cRef);

        if ( cRef == 0 )
        {
            delete this;
        }

        return cRef;
    }

public:
    U8CSTR   __stdcall name() const override { return m_name; }
    uint32_t __stdcall flag() const override { return m_flag; }

protected:
    ULONG    m_cRef = 0;
    uint32_t m_flag = 0;
    char8_t  m_name[MAX_DATA_NAME_LENGTH];

private:
    CData(const CData&)             = delete;
    CData(CData&&)                  = delete;
    CData& operator =(const CData&) = delete;
    CData& operator =(CData&&)      = delete;
};

//---------------------------------------------------------------------------//

template<typename T> class Data : public CData
{
public:
    explicit Data(const T& data) : Data(nullptr, data)
    {
    }

    explicit Data(U8CSTR name, const T& data) : CData(name)
    {
        m_data = data;
        console_outA("Data<T>::ctor(%s)", m_name);
    }

    ~Data() override
    {
        console_outA("Data<T>::dtor(%s)", m_name);
    }

public:
    size_t    __stdcall size() const override { return sizeof(T); }
    U8CSTR    __stdcall type() const override { return (U8CSTR)"UNKNOWN"; }
    uintptr_t __stdcall get()  const override { return (uintptr_t)m_data; }
    HRESULT   __stdcall set(const uintptr_t value) override { m_data = (T)value; return S_OK; }

protected:
    T m_data;
};

//---------------------------------------------------------------------------//

template<> class Data<void> : public CData
{
public:
    explicit Data(U8CSTR name = nullptr) : CData(name)
    {
        console_outA("Data<void>::ctor(%s)", m_name);
    }

    ~Data() override
    {
        console_outA("Data<void>::dtor(%s)", m_name);
    }

public:
    size_t    __stdcall size() const override { return 0; }
    U8CSTR    __stdcall type() const override { return (U8CSTR)"void"; }
    uintptr_t __stdcall get()  const override { return 0; }
    HRESULT   __stdcall set(const uintptr_t value) override { return S_FALSE; }
};

//---------------------------------------------------------------------------//

template<> class Data<U8CSTR> : public CData
{
public:
    explicit Data(U8CSTR str) : Data(nullptr, str)
    {
    }

    explicit Data(U8CSTR name, U8CSTR str) : CData(name)
    {
        m_size = 1 + lstrlenA((const char*)str);
        m_data = new char8_t[m_size * sizeof(char8_t)];

        ::StringCchCopyA((char*)m_data, m_size, (const char*)str);

        console_outA("Data<U8CSTR>::ctor(%s) \"%s\"", m_name, m_data);
    }

    ~Data() override
    {
        console_outA("Data<U8CSTR>::dtor(%s) \"%s\"", m_name, m_data);
        if ( m_data )
        {
            delete m_data;
            m_data = nullptr;
        }
    }

public:
    size_t    __stdcall size() const override { return m_size; }
    U8CSTR    __stdcall type() const override { return (U8CSTR)"U8CSTR"; }
    uintptr_t __stdcall get()  const override { return (uintptr_t)m_data; }
    HRESULT   __stdcall set(const uintptr_t value) override
    {
        const auto str = (U8CSTR)value;
        if ( nullptr == str )
        {
            return E_POINTER;
        }
        if ( m_data )
        {
            delete m_data;
        }

        m_size = 1 + lstrlenA((const char*)str);
        m_data = new char8_t[m_size * sizeof(char8_t)];

        ::StringCchCopyA((char*)m_data, m_size, (const char*)str);

        return S_OK;
    }

protected:
    char8_t* m_data;
    size_t   m_size;
};

//---------------------------------------------------------------------------//

template<> class Data<void*> : public CData
{
public:
    explicit Data(void* data, size_t size) : Data(nullptr, data, size)
    {
    }

    explicit Data(U8CSTR name, void* data, size_t size) : CData(name)
    {
        m_data = data;
        m_size = size;
        console_outA("Data<void*>::ctor(%s)", m_name);
    }

    ~Data() override
    {
        console_outA("Data<void*>::dtor(%s)", m_name);
    }

public:
    size_t    __stdcall size() const override { return m_size; }
    U8CSTR    __stdcall type() const override { return (U8CSTR)"void*"; }
    uintptr_t __stdcall get()  const override { return (uintptr_t)m_data; }
    HRESULT   __stdcall set(const uintptr_t value) override { return S_FALSE; }

protected:
    void*  m_data;
    size_t m_size;
};

//---------------------------------------------------------------------------//

// Data.hpp