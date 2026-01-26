#pragma once

#include <flecs.h>

namespace flua::ecs
{
struct GuardedEcsIterator
{
    explicit GuardedEcsIterator(const ecs_iter_t& iterator) : m_iterator(iterator) {}
    ~GuardedEcsIterator() { if (!m_finished) ecs_iter_fini(&m_iterator); }

    void finish() { m_finished = true; }
    ecs_iter_t* operator->() { return &m_iterator; }
    const ecs_iter_t* operator->() const { return &m_iterator; }
    ecs_iter_t& operator*() { return m_iterator; }
    const ecs_iter_t& operator*() const { return m_iterator; }

private:
    ecs_iter_t m_iterator;
    bool m_finished = false;
};
}
