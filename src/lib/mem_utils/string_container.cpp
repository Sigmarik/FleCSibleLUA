#include "string_container.h"


flua::mem_utils::StringContainer& flua::mem_utils::StringContainer::GetInstance()
{
    thread_local StringContainer container;
    return container;
}

flua::mem_utils::StringContainer::Pointer::Pointer(const Pointer& other)
{
    m_cell = other.m_cell;
    ++m_cell->useCount;
}

flua::mem_utils::StringContainer::Pointer& flua::mem_utils::StringContainer::Pointer::operator=(const Pointer& other)
{
    if (&other == this) return *this;
    m_cell = other.m_cell;
    ++m_cell->useCount;
    return *this;
}

flua::mem_utils::StringContainer::Pointer::Pointer(Pointer&& other) noexcept
{
    m_cell = other.m_cell;
    ++m_cell->useCount;
}

flua::mem_utils::StringContainer::Pointer& flua::mem_utils::StringContainer::Pointer::operator=(
    Pointer&& other) noexcept
{
    m_cell = other.m_cell;
    ++m_cell->useCount;
    return *this;
}

flua::mem_utils::StringContainer::Pointer::~Pointer()
{
    --m_cell->useCount;
    if (m_cell->useCount == 0)
    {
        StringContainer& instance = StringContainer::GetInstance();
        auto found = instance.m_hashes.find(m_cell->object);
        delete m_cell;
        instance.m_hashes.erase(found);
        instance.m_cells.erase(m_cell);
    }
}

flua::mem_utils::StringContainer::Pointer::Pointer(const std::string& content)
{
    std::string copy = content;
    *this = Pointer(std::move(copy));
}

flua::mem_utils::StringContainer::Pointer::Pointer(std::string&& content)
{
    std::string thisContent = std::move(content);
    StringContainer& instance = StringContainer::GetInstance();
    auto found = instance.m_hashes.find(thisContent);
    if (found != instance.m_hashes.end())
    {
        m_cell = found->second;
        ++m_cell->useCount;
    }
    else
    {
        m_cell = new Cell();
        m_cell->object = std::move(thisContent);
        m_cell->useCount = 1;
        instance.m_hashes[thisContent] = m_cell;
        instance.m_cells[m_cell] = m_cell;
    }
}
