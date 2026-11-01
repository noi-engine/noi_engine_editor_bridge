#include "component_handler.hpp"
#include "properties/property_value.hpp"

#include <noi_engine/builtin/components/orbit_camera.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::orbit_camera;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& cam = c.world.get<orbit_camera>(c.entity);
            emitter.emit_vec3("target", cam.target, false);
            emitter.emit_float("distance", cam.distance, false);
            emitter.emit_float("yaw", cam.yaw, false);
            emitter.emit_float("pitch", cam.pitch, false);
            emitter.emit_float("sensitivity", cam.sensitivity, false);
            emitter.emit_float("zoom_speed", cam.zoom_speed, false);
            emitter.emit_float("min_distance", cam.min_distance, false);
            emitter.emit_float("max_distance", cam.max_distance, false);
            emitter.emit_float("min_pitch", cam.min_pitch, false);
            emitter.emit_float("max_pitch", cam.max_pitch, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            auto& cam = c.world.get<orbit_camera>(c.entity);

            if (name == "target" && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_VEC3)
            {
                cam.target = vec3_of(value);
                return true;
            }
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }

            const float v = value.number[0];
            if (name == "distance") { cam.distance = v; }
            else if (name == "yaw") { cam.yaw = v; }
            else if (name == "pitch") { cam.pitch = v; }
            else if (name == "sensitivity") { cam.sensitivity = v; }
            else if (name == "zoom_speed") { cam.zoom_speed = v; }
            else if (name == "min_distance") { cam.min_distance = v; }
            else if (name == "max_distance") { cam.max_distance = v; }
            else if (name == "min_pitch") { cam.min_pitch = v; }
            else if (name == "max_pitch") { cam.max_pitch = v; }
            else { return false; }
            return true;
        }
    }

    auto make_orbit_camera_handler() -> component_handler
    {
        return {
            .type = "orbit_camera",
            .has = component_ops::has<orbit_camera>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<orbit_camera>,
            .remove = component_ops::remove<orbit_camera>,
            .copy = component_ops::copy<orbit_camera>,
        };
    }
}
