#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/culling.hpp>

// "culling" on the scene camera: draw (and optionally simulate) only what the camera sees, plus
// a margin (see noi_engine/core/components/culling.hpp).
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::culling;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& value = c.world.get<culling>(c.entity);
            emitter.emit_bool("enabled", value.enabled, false);
            emitter.emit_float("margin", value.margin, false);
            emitter.emit_bool("simulate_offscreen", value.simulate_offscreen, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            auto& settings = c.world.get<culling>(c.entity);
            if (name == "margin" && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT && value.number[0] >= 0.0f)
            {
                settings.margin = value.number[0];
                return true;
            }
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_BOOL)
            {
                return false;
            }
            if (name == "enabled") { settings.enabled = value.flag != 0; }
            else if (name == "simulate_offscreen") { settings.simulate_offscreen = value.flag != 0; }
            else { return false; }
            return true;
        }

        // Culling follows the scene camera's view: it only makes sense there.
        auto add(const component_context& c) -> bool
        {
            return c.entity == c.editor.camera_entity && component_ops::add<culling>(c);
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            const auto& value = c.world.get<culling>(c.entity);
            components.push_back({{"type", "culling"},
                                  {"properties", {{"enabled", value.enabled},
                                                  {"margin", value.margin},
                                                  {"simulate_offscreen", value.simulate_offscreen}}}});
        }
    }

    auto make_culling_handler() -> component_handler
    {
        return {
            .type = "culling",
            .has = component_ops::has<culling>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add,
            .remove = component_ops::remove<culling>,
            .copy = component_ops::copy<culling>,
            .save = save,
        };
    }
}
