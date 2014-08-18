// SRWLock.hpp

#pragma once

//---------------------------------------------------------------------------//

#include <windows.h>

//---------------------------------------------------------------------------//

namespace SRWLock {

//---------------------------------------------------------------------------//

struct LockHolder final
{
private:
    SRWLOCK m_srwl;

public:
    LockHolder()
    {
        ::InitializeSRWLock(&m_srwl);
    }

    LockHolder(LockHolder&& rhs)
    {
        m_srwl     = rhs.m_srwl;
        rhs.m_srwl = SRWLOCK();
    }

    LockHolder& operator =(LockHolder&& rhs)
    {
        m_srwl     = rhs.m_srwl;
        rhs.m_srwl = SRWLOCK();

        return *this;
    }

    ~LockHolder()
    {
    }

public:
    SRWLOCK* ptr() const { return (SRWLOCK*)&m_srwl; }

private:
    LockHolder(const LockHolder&)             = delete;
    LockHolder& operator =(const LockHolder&) = delete;
};

//---------------------------------------------------------------------------//

struct ReadLock final
{
private:
    SRWLOCK* p_srwl;

public:
    ReadLock(LockHolder& holder)
    {
        p_srwl = holder.ptr();
        ::AcquireSRWLockShared(p_srwl);
    }

    ~ReadLock()
    {
        ::ReleaseSRWLockShared(p_srwl);
    }

private:
    ReadLock(const ReadLock&)             = delete;
    ReadLock(ReadLock&&)                  = delete;
    ReadLock& operator =(const ReadLock&) = delete;
    ReadLock& operator =(ReadLock&&)      = delete;
};

//---------------------------------------------------------------------------//

struct WriteLock final
{
private:
    SRWLOCK* p_srwl;

public:
    WriteLock(LockHolder& holder)
    {
        p_srwl = holder.ptr();
        ::AcquireSRWLockExclusive(p_srwl);
    }

    ~WriteLock()
    {
        ::ReleaseSRWLockExclusive(p_srwl);
    }

private:
    WriteLock(const WriteLock&)             = delete;
    WriteLock(WriteLock&&)                  = delete;
    WriteLock& operator =(const WriteLock&) = delete;
    WriteLock& operator =(WriteLock&&)      = delete;
};

//---------------------------------------------------------------------------//

} // namespace SRWLock

//---------------------------------------------------------------------------//

// SRWLock.hpp