#pragma once

#include "noi_engine_editor_bridge/bridge_api.h"

#include <glm/glm.hpp>
#include <string>

// Small readers for the values the editor sends in set_*_property calls.
namespace noi_engine_editor_bridge_detail
{
    inline auto text_of(const noi_engine_editor_bridge_property_value& value) -> std::string
    {
        return value.text ? value.text : "";
    }

    inline auto vec3_of(const noi_engine_editor_bridge_property_value& value) -> glm::vec3
    {
        return {value.number[0], value.number[1], value.number[2]};
    }

    inline auto vec4_of(const noi_engine_editor_bridge_property_value& value) -> glm::vec4
    {
        return {value.number[0], value.number[1], value.number[2], value.number[3]};
    }
}
