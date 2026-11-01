#pragma once

#include "component_handler.hpp"

#include <nlohmann/json.hpp>

namespace noi_engine_editor_bridge_detail
{
    // The scene format stores mesh_renderer_properties inside the mesh_renderer component
    // ("material_properties"), so mesh_renderer.cpp's save() calls this. Returns an empty
    // object when the entity has no overrides.
    [[nodiscard]] auto mesh_renderer_properties_json(const component_context& c) -> nlohmann::json;
}
