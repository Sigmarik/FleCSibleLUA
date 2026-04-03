#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace flua::mem_utils
{

class StringContainer
{
    struct Cell;

public:
    StringContainer(const StringContainer&) = delete;
    StringContainer(StringContainer&&) = delete;
    StringContainer& operator=(const StringContainer&) = delete;
    StringContainer& operator=(StringContainer&&) = delete;

    static StringContainer& GetInstance();

    class Pointer
    {
    public:
        Pointer(const Pointer& other);
        Pointer& operator=(const Pointer& other);

        Pointer(Pointer&& other) noexcept;
        Pointer& operator=(Pointer&& other) noexcept;

        ~Pointer();

        explicit Pointer(const std::string& content);
        explicit Pointer(std::string&& content);

        std::string* operator->() { return &m_cell->object; }
        std::string& operator*() { return m_cell->object; }

        const std::string* operator->() const { return &m_cell->object; }
        const std::string& operator*() const { return m_cell->object; }

        bool operator<(const Pointer& other) const { return m_cell < other.m_cell; }
        bool operator>(const Pointer& other) const { return m_cell > other.m_cell; }

        bool operator==(const Pointer& other) const { return m_cell == other.m_cell; }

    private:
        Cell* m_cell = nullptr;
    };

private:
    StringContainer() = default;

    struct Cell
    {
        unsigned useCount = 0;
        std::string object;
    };

    std::unordered_map<std::string, Cell*> m_hashes;
    std::map<Cell*, Cell*> m_cells;
};

using PointerMappedString = StringContainer::Pointer;

}
