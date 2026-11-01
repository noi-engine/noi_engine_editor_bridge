#include "component_handler.hpp"
#include "persistence/json_utils.hpp"
#include "properties/property_value.hpp"

#include <noi_engine/core/components/dirty.hpp>
#include <noi_engine/core/components/transform.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::transform;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& t = c.world.get<transform>(c.entity);
            emitter.emit_vec3("position", t.position, false);
            emitter.emit_vec3("rotation", t.rotation, false);
            emitter.emit_vec3("scale", t.scale, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_VEC3)
            {
                return false;
            }

            auto& t = c.world.get<transform>(c.entity);
            const auto v = vec3_of(value);

            if (name == "position") { t.position = v; }
            else if (name == "rotation") { t.rotation = v; }
            else if (name == "scale") { t.scale = v; }
            else { return false; }

            c.world.add<noi_engine::dirty<transform>>(c.entity, noi_engine::dirty<transform>{});
            return true;
        }

        auto add(const component_context& c) -> bool
        {
            if (c.world.has<transform>(c.entity))
            {
                return false;
            }
            c.world.add_transform(c.entity, {});
            return true;
        }

        // add_transform() (not add<transform>) so the copy also gets its world_matrix: without
        // it, transform_system crashes as soon as the copy's transform is marked dirty, and the
        // renderer never draws it.
        auto copy(const component_context& from, const noi_engine::entity to) -> void
        {
            if (from.world.has<transform>(from.entity))
            {
                from.world.add_transform(to, from.world.get<transform>(from.entity));
            }
        }

        auto save(const component_context& c, const bool is_camera, nlohmann::json& components) -> void
        {
            const auto& t = c.world.get<transform>(c.entity);
            nlohmann::json props;
            props["position"] = vec3_json(t.position);
            props["rotation"] = vec3_json(t.rotation);
            // set_scene_2d() overwrites the camera entity's scale to size the border gizmo -
            // never persist that, always write identity scale back for the camera.
            props["scale"] = is_camera ? vec3_json(glm::vec3{1.0f}) : vec3_json(t.scale);
            components.push_back({{"type", "transform"}, {"properties", props}});
        }
    }

    auto make_transform_handler() -> component_handler
    {
        return {
            .type = "transform",
            .has = component_ops::has<transform>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add,
            .remove = component_ops::remove<transform>,
            .copy = copy,
            .save = save,
        };
    }
}
