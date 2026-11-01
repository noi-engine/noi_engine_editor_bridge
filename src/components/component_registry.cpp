#include "component_registry.hpp"

#include <array>

namespace noi_engine_editor_bridge_detail
{
    // One per file in components/.
    auto make_box_collider_handler() -> component_handler;
    auto make_camera_2d_handler() -> component_handler;
    auto make_camera_3d_handler() -> component_handler;
    auto make_folder_handler() -> component_handler;
    auto make_gravity_handler() -> component_handler;
    auto make_mesh_renderer_handler() -> component_handler;
    auto make_mesh_renderer_properties_handler() -> component_handler;
    auto make_name_component_handler() -> component_handler;
    auto make_orbit_camera_handler() -> component_handler;
    auto make_render_layer_handler() -> component_handler;
    auto make_script_component_handler() -> component_handler;
    auto make_transform_handler() -> component_handler;
    auto make_world_matrix_handler() -> component_handler;

    auto component_handlers() -> std::span<const component_handler>
    {
        static const std::array handlers{
            make_box_collider_handler(),
            make_camera_2d_handler(),
            make_camera_3d_handler(),
            make_folder_handler(),
            make_gravity_handler(),
            make_mesh_renderer_handler(),
            make_mesh_renderer_properties_handler(),
            make_name_component_handler(),
            make_orbit_camera_handler(),
            make_render_layer_handler(),
            make_script_component_handler(),
            make_transform_handler(),
            make_world_matrix_handler(),
        };
        return handlers;
    }

    auto find_component_handler(const std::string_view type) -> const component_handler*
    {
        for (const auto& handler : component_handlers())
        {
            if (handler.type == type)
            {
                return &handler;
            }
        }
        return nullptr;
    }
}
