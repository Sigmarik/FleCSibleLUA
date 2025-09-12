#pragma once

#include <utility>

namespace flua::mem_utils
{

template <class T>
class CopyMovePtr {
public:
    CopyMovePtr(const CopyMovePtr& ptr)
        : m_data(new T(*ptr.m_data))
    {}
    CopyMovePtr(CopyMovePtr&& ptr)
        : m_data(ptr.m_data)
    {
        ptr.m_data = nullptr;
    }

    template <class ...Args>
    explicit CopyMovePtr(Args&&... args)
        : m_data(new T(std::forward<Args>(args)...))
    {}

    CopyMovePtr& operator=(const CopyMovePtr& ptr)
    {
        m_data = new T(*ptr.m_data);
    }
    CopyMovePtr& operator=(CopyMovePtr&& ptr)
    {
        m_data = ptr.m_data;
        ptr.m_data = nullptr;
    }

    ~CopyMovePtr()
    {
        if (m_data) delete m_data;
    }

    T& operator*() { return *m_data; }
    const T& operator*() const { return *m_data; }

    T& operator->() { return *m_data; }
    const T& operator->() const { return *m_data; }

private:
    T* m_data;
};

}
