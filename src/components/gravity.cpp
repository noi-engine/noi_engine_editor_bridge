#include "component_handler.hpp"

#include <noi_engine/builtin/components/gravity.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::gravity;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& g = c.world.get<gravity>(c.entity);
            emitter.emit_float("acceleration_x", g.acceleration.x, false);
            emitter.emit_float("acceleration_y", g.acceleration.y, false);
            emitter.emit_bool("enabled", g.enabled, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            auto& g = c.world.get<gravity>(c.entity);

            if (name == "enabled" && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_BOOL)
            {
                g.enabled = value.flag != 0;
                return true;
            }
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }

            if (name == "acceleration_x") { g.acceleration.x = value.number[0]; }
            else if (name == "acceleration_y") { g.acceleration.y = value.number[0]; }
            else { return false; }
            return true;
        }
    }

    auto make_gravity_handler() -> component_handler
    {
        return {
            .type = "gravity",
            .has = component_ops::has<gravity>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<gravity>,
            .remove = component_ops::remove<gravity>,
            .copy = component_ops::copy<gravity>,
        };
    }
}
