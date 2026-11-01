#pragma once

#include "component_handler.hpp"

#include <span>
#include <string_view>

namespace noi_engine_editor_bridge_detail
{
    // Every component type the editor knows about, in alphabetical order (the order the
    // Hierarchy lists an entity's components in).
    [[nodiscard]] auto component_handlers() -> std::span<const component_handler>;

    // nullptr if the editor doesn't know this component type.
    [[nodiscard]] auto find_component_handler(std::string_view type) -> const component_handler*;
}
