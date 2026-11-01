#include "component_handler.hpp"

#include <noi_engine/core/components/camera_3d.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::camera_3d;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& cam = c.world.get<camera_3d>(c.entity);
            emitter.emit_float("fov", cam.fov, false);
            emitter.emit_float("near_plane", cam.near_plane, false);
            emitter.emit_float("far_plane", cam.far_plane, false);
            emitter.emit_float("aspect_ratio", cam.aspect_ratio, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }

            auto& cam = c.world.get<camera_3d>(c.entity);
            const float v = value.number[0];

            if (name == "fov") { cam.fov = v; }
            else if (name == "near_plane") { cam.near_plane = v; }
            else if (name == "far_plane") { cam.far_plane = v; }
            else if (name == "aspect_ratio") { cam.aspect_ratio = v; }
            else { return false; }
            return true;
        }
    }

    namespace
    {
        // The scene camera is a 2D camera: a 3D camera component on it would replace its view.
        auto add_unless_scene_camera(const component_context& c) -> bool
        {
            return c.entity != c.editor.camera_entity && component_ops::add<noi_engine::camera_3d>(c);
        }
    }

    auto make_camera_3d_handler() -> component_handler
    {
        return {
            .type = "camera_3d",
            .has = component_ops::has<camera_3d>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add_unless_scene_camera,
            .remove = component_ops::remove<camera_3d>,
            .copy = component_ops::copy<camera_3d>,
        };
    }
}
