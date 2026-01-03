#include "comp_map.h"

namespace flua::cmp_info
{
extern const std::unordered_map<std::string, EntityMemberAccessor> ENTITY_MEMBER_MAP{};

extern const std::unordered_map<std::string, EntityComponentChecker> ENTITY_COMPONENT_CHECKERS{};
}
