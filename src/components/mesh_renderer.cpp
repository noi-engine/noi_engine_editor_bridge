#include "mesh_renderer_properties.hpp"
#include "properties/property_value.hpp"

#include <noi_engine/core/components/mesh_renderer.hpp>
#include <noi_engine/core/model/material.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::mesh_renderer;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& mr = c.world.get<mesh_renderer>(c.entity);
            const std::string mesh_label{c.resources.get_label<noi_engine::mesh>(mr.m_mesh)};
            const std::string material_label{c.resources.get_label<noi_engine::material>(mr.m_material)};
            emitter.emit_resource_ref("mesh", mesh_label, NOI_ENGINE_EDITOR_BRIDGE_RESOURCE_MESH, false);
            emitter.emit_resource_ref("material", material_label, NOI_ENGINE_EDITOR_BRIDGE_RESOURCE_MATERIAL, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_RESOURCE_REF)
            {
                return false;
            }

            auto& mr = c.world.get<mesh_renderer>(c.entity);
            const auto label = text_of(value);

            if (name == "mesh")
            {
                mr.m_mesh = c.resources.get_handle<noi_engine::mesh>(label);
                return true;
            }
            if (name == "material")
            {
                mr.m_material = c.resources.get_handle<noi_engine::material>(label);
                return true;
            }
            return false;
        }

        auto add(const component_context& c) -> bool
        {
            if (c.world.has<mesh_renderer>(c.entity))
            {
                return false;
            }
            // A default mesh_renderer{} has both handles at {id=0, generation=0}, which only
            // happens to be the builtin quad/color material because they are loaded first
            // (see game::init_builtin_resources()). Look them up by label instead.
            c.world.add<mesh_renderer>(c.entity, {
                                           .m_mesh = c.resources.get_handle<noi_engine::mesh>("meshes:quad_2d"),
                                           .m_material = c.resources.get_handle<noi_engine::material>("materials:color")
                                       });
            return true;
        }

        auto save(const component_context& c, const bool is_camera, nlohmann::json& components) -> void
        {
            // The camera entity's mesh_renderer is the injected red border gizmo (set_scene_2d())
            // - editor-only visualization, never part of the authored scene.
            if (is_camera)
            {
                return;
            }

            const auto& mr = c.world.get<mesh_renderer>(c.entity);
            nlohmann::json props;
            props["mesh"] = std::string(c.resources.get_label<noi_engine::mesh>(mr.m_mesh));
            props["material"] = std::string(c.resources.get_label<noi_engine::material>(mr.m_material));

            if (auto material_properties = mesh_renderer_properties_json(c); !material_properties.empty())
            {
                props["material_properties"] = material_properties;
            }

            components.push_back({{"type", "mesh_renderer"}, {"properties", props}});
        }
    }

    auto make_mesh_renderer_handler() -> component_handler
    {
        return {
            .type = "mesh_renderer",
            .has = component_ops::has<mesh_renderer>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add,
            .remove = component_ops::remove<mesh_renderer>,
            .copy = component_ops::copy<mesh_renderer>,
            .save = save,
        };
    }
}
