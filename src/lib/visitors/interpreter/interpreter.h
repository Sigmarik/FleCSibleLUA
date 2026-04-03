#pragma once

#include <memory>
#include <set>
#include <random>

#include "ast/visitor.h"
#include "types/data_types.h"
#include "flecsible_lua_api.h"
#include "component_map/comp_map.h"
#include "meta/remap.h"

namespace flua::lib
{
void print(FluaState&);
}

namespace flua::lib::misc
{
void pcall(FluaState&);
void error(FluaState&);
}

namespace flua::vst
{
using namespace flua;

class Interpreter final : public ast::Visitor
{
public:
    struct LuaRuntimeError : std::exception
    {
        LuaRuntimeError(const ast::INode& node, const std::string& what) : what(what), where(node.getPos()) {}

        std::string what;
        parser::CharacterPos where;
        std::optional<data::GenericValue> data{};
    };

    Interpreter(std::ostream& outStream, std::ostream& errStream, flecs::world* world)
        : m_errStream(errStream), m_outStream(outStream), m_world(world)
    {
        m_stack.emplace_back();
        m_componentIds = cmp_info::get_cached_component_ids(*world);
    }

    ~Interpreter() override;

    template <class ValueT>
    void setGlobal(const mem_utils::PointerMappedString& name, const ValueT& value)
    {
        data::GenericValue* found = getGlobalValueByName(name);
        if (!found) return;
        *found = data::GenericValue(value);
    }

    template <class ValueT>
    ValueT getGlobal(const mem_utils::PointerMappedString& name) const
    {
        const data::GenericValue* found = getGlobalValueByName(name);
        return std::get<ValueT>(*found);
    }

    template <class ValueT>
    bool isGlobalOfType(const mem_utils::PointerMappedString& name) const
    {
        const data::GenericValue* found = getGlobalValueByName(name);
        if (!found) return false;
        return std::holds_alternative<ValueT>(*found);
    }

    [[nodiscard]] bool fallen() const { return m_fallen; }

protected:
    using Visitor::visit;

    void visit(ast::Program& node) override;
    void visit(ast::Function& node) override;
    void visit(ast::System& node) override;
    void visit(ast::WhileLoop& node) override;
    void visit(ast::ForLoopNumeric& node) override;
    void visit(ast::ForLoopGeneric& node) override;
    void visit(ast::Query& node) override;
    void visit(ast::RepeatUntil& node) override;
    void visit(ast::Branch& node) override;
    void visit(ast::FunctionCall& node) override;
    void visit(ast::UnaryOperator& node) override;
    void visit(ast::BinaryOperator& node) override;
    void visit(ast::FieldRequest& node) override;
    void visit(ast::IndexRequest& node) override;
    void visit(ast::Constant& node) override;
    void visit(ast::MakeTable& node) override;
    void visit(ast::Variable& node) override;
    void visit(ast::Assignment& node) override;
    void visit(ast::LocalAssignment& node) override;
    void visit(ast::Return& node) override;
    void visit(ast::Break& node) override;
    void visit(ast::Continue& node) override;

    friend class FluaState;

private:
    template <class VecN>
    static std::vector<double> extract_vector_components(const VecN& vec,
        const std::string& indices, ast::INode& node);

    friend void lib::print(FluaState& state);
    friend void lib::misc::pcall(FluaState&);
    friend void lib::misc::error(FluaState&);

    void performFixedTypeAssignment(ast::Assignment& node, cmp_info::GenericComponentPtr ptr,
                                    data::GenericValue& value);

    void indexEntity(ast::IndexRequest& node, data::Entity& entity, const mem_utils::PointerMappedString& index);

    void indexEntityComponent(ast::IndexRequest& node, data::EntityComponent& component,
        const mem_utils::PointerMappedString& index);

    void visitTransparentBlock(std::deque<ast::NodePtr>& nodes);

    void executeFunction(const ast::INode& node, data::Function& function, std::deque<ast::NodePtr>& args);

    void executeFunction(ast::Function& function);

    void runAnyFunction(data::Function& func, std::vector<data::GenericValue>& arguments);
    void runLuaFunction(data::LuaFunction& function, std::vector<data::GenericValue>& args);

    FluaState generatePublicState();

    void printError(const LuaRuntimeError& err);

    ecs_query_desc_t makeEcsQueryDesc(const ast::EcsEntityFilter& filter, ast::INode& node);
    ecs_query_t* makeEcsQuery(const ast::EcsEntityFilter& filter, ast::INode& node);

    data::GenericValue* getGlobalValueByName(const mem_utils::PointerMappedString& name);
    const data::GenericValue* getGlobalValueByName(const mem_utils::PointerMappedString& name) const;

    struct NameQueryPair
    {
        mem_utils::PointerMappedString entityName{};
        ecs_query_t* query = nullptr;
    };

    void runBodyWithinQueries(std::vector<NameQueryPair>& queries, std::deque<ast::NodePtr>& body,
        unsigned iterId);

    void prepareAndRunSystem(ast::System& system, ecs_iter_t* systemIt);

    static void system_runner(ecs_iter_t *it);

    struct Frame
    {
        Frame() = default;

        Frame(const Frame&) = delete;

        Frame(Frame&&) = default;

        std::map<mem_utils::PointerMappedString, mem_utils::CopyMovePtr<data::GenericValue> > varNameMap{};
        bool transparent = false;
    };

    struct NamespaceHolder
    {
        explicit NamespaceHolder(std::vector<Frame>& stack, bool transparent = true)
            : m_stack(&stack)
        {
            m_stack->emplace_back();
            m_stack->back().transparent = transparent;
        }

        ~NamespaceHolder() { m_stack->pop_back(); }

    private:
        std::vector<Frame>* m_stack;
    };

    struct RegisteredSystemInfo
    {
        Interpreter* interpreter = nullptr;
        ast::System* luaSystem = nullptr;
    };

    static std::map<flecs::entity_t, RegisteredSystemInfo> s_interpreterSystems;

    std::vector<Frame> m_stack{};

    data::ValueSequence m_returnedValue{};

    bool m_breaking = false;
    bool m_continuing = false;
    bool m_returning = false;
    bool m_inLocalAssignment = false;

    bool m_fallen = false;

    std::ostream& m_errStream;
    std::ostream& m_outStream;

    std::vector<data::GenericValue> m_externalFunctionInputs{};
    std::vector<data::GenericValue> m_externalFunctionOutputs{};

    flecs::world* m_world{};

    std::map<mem_utils::PointerMappedString, ecs_id_t> m_componentIds{};

    using QueryArray = std::vector<NameQueryPair>;
    std::map<ast::INode*, QueryArray> m_nodeQueries{};

    std::set<ecs_entity_t> m_ownedSystems{};

    const ast::INode* m_functionCaller = nullptr;

    std::mt19937 m_rng{std::random_device{}()};
};
}
