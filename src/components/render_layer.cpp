#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/render_layer.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::render_layer;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            emitter.emit_float("order", static_cast<float>(c.world.get<render_layer>(c.entity).order), false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (name != "order" || value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }
            c.world.get<render_layer>(c.entity).order = static_cast<int>(value.number[0]);
            return true;
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            const auto& r = c.world.get<render_layer>(c.entity);
            components.push_back({{"type", "render_layer"}, {"properties", {{"order", r.order}}}});
        }
    }

    auto make_render_layer_handler() -> component_handler
    {
        return {
            .type = "render_layer",
            .has = component_ops::has<render_layer>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<render_layer>,
            .remove = component_ops::remove<render_layer>,
            .copy = component_ops::copy<render_layer>,
            .save = save,
        };
    }
}
