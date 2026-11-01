#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/builtin/components/box_collider.hpp>
#include <noi_engine/core/components/transform.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::box_collider;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& bc = c.world.get<box_collider>(c.entity);
            emitter.emit_float("half_width", bc.half_extents.x, false);
            emitter.emit_float("half_height", bc.half_extents.y, false);
            emitter.emit_bool("is_static", bc.is_static, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            auto& bc = c.world.get<box_collider>(c.entity);

            if (name == "is_static" && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_BOOL)
            {
                bc.is_static = value.flag != 0;
                return true;
            }
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }

            if (name == "half_width") { bc.half_extents.x = value.number[0]; }
            else if (name == "half_height") { bc.half_extents.y = value.number[0]; }
            else { return false; }
            return true;
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            const auto& bc = c.world.get<box_collider>(c.entity);
            nlohmann::json props;
            props["half_extents"] = nlohmann::json::array({bc.half_extents.x, bc.half_extents.y});
            props["is_static"] = bc.is_static;
            components.push_back({{"type", "box_collider"}, {"properties", props}});
        }
    }

    namespace
    {
        // Physics moves the transform: add one if the entity has none.
        auto add_with_transform(const component_context& c) -> bool
        {
            if (c.world.has<box_collider>(c.entity))
            {
                return false;
            }
            if (!c.world.has<noi_engine::transform>(c.entity))
            {
                c.world.add_transform(c.entity, {});
            }
            // Sized to the sprite: the entity's quad is its scale (follows later scale edits,
            // see transform.cpp).
            const auto& scale = c.world.get<noi_engine::transform>(c.entity).scale;
            c.world.add<box_collider>(c.entity, box_collider{.half_extents = {scale.x * 0.5f, scale.y * 0.5f}});
            return true;
        }
    }

    auto make_box_collider_handler() -> component_handler
    {
        return {
            .type = "box_collider",
            .has = component_ops::has<box_collider>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add_with_transform,
            .remove = component_ops::remove<box_collider>,
            .copy = component_ops::copy<box_collider>,
            .save = save,
        };
    }
}
