#include "mesh_renderer_properties.hpp"
#include "persistence/json_utils.hpp"
#include "properties/property_value.hpp"
#include "properties/resource_ref.hpp"

#include <noi_engine/core/components/mesh_renderer_properties.hpp>

// Per-entity material overrides, exposed to the editor as "param:<name>", "color:<name>"
// and "texture:<name>" properties.
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::mesh_renderer_properties;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& mrp = c.world.get<mesh_renderer_properties>(c.entity);

            for (const auto& [key, v] : mrp.m_parameters)
            {
                emitter.emit_float(("param:" + key).c_str(), v, false);
            }
            for (const auto& [key, v] : mrp.m_colors)
            {
                emitter.emit_vec4(("color:" + key).c_str(), v, false);
            }
            for (const auto& [key, handle] : mrp.m_textures)
            {
                const std::string label{c.resources.get_label<noi_engine::texture>(handle)};
                emitter.emit_resource_ref(("texture:" + key).c_str(), label, NOI_ENGINE_EDITOR_BRIDGE_RESOURCE_TEXTURE,
                                          false);
            }
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            auto& mrp = c.world.get<mesh_renderer_properties>(c.entity);

            if (name.starts_with("param:") && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                mrp.m_parameters[name.substr(6)] = value.number[0];
                return true;
            }
            if (name.starts_with("color:") && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_VEC4)
            {
                mrp.m_colors[name.substr(6)] = vec4_of(value);
                return true;
            }
            if (name.starts_with("texture:") && value.type == NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_RESOURCE_REF)
            {
                noi_engine::resource_handle<noi_engine::texture> texture{};
                if (!find_resource(c.resources, text_of(value), texture))
                {
                    return false;
                }
                mrp.m_textures[name.substr(8)] = texture;
                return true;
            }
            return false;
        }

        auto remove_property(const component_context& c, const std::string& name) -> bool
        {
            auto& mrp = c.world.get<mesh_renderer_properties>(c.entity);

            if (name.starts_with("param:"))
            {
                return mrp.m_parameters.erase(name.substr(6)) > 0;
            }
            if (name.starts_with("color:"))
            {
                return mrp.m_colors.erase(name.substr(6)) > 0;
            }
            if (name.starts_with("texture:"))
            {
                return mrp.m_textures.erase(name.substr(8)) > 0;
            }
            return false;
        }
    }

    auto mesh_renderer_properties_json(const component_context& c) -> nlohmann::json
    {
        nlohmann::json material_properties;
        if (!c.world.has<mesh_renderer_properties>(c.entity))
        {
            return material_properties;
        }

        const auto& mrp = c.world.get<mesh_renderer_properties>(c.entity);

        if (!mrp.m_parameters.empty())
        {
            auto parameters = nlohmann::json::array();
            for (const auto& [key, v] : mrp.m_parameters)
            {
                parameters.push_back({{"name", key}, {"value", v}});
            }
            material_properties["parameters"] = parameters;
        }

        if (!mrp.m_colors.empty())
        {
            auto colors = nlohmann::json::array();
            for (const auto& [key, v] : mrp.m_colors)
            {
                colors.push_back({{"name", key}, {"value", to_hex_string(v)}});
            }
            material_properties["colors"] = colors;
        }

        if (!mrp.m_textures.empty())
        {
            auto textures = nlohmann::json::array();
            for (const auto& [key, handle] : mrp.m_textures)
            {
                textures.push_back({
                    {"name", key},
                    {"value", std::string(c.resources.get_label<noi_engine::texture>(handle))}
                });
            }
            material_properties["textures"] = textures;
        }

        return material_properties;
    }

    auto make_mesh_renderer_properties_handler() -> component_handler
    {
        return {
            .type = "mesh_renderer_properties",
            .has = component_ops::has<mesh_renderer_properties>,
            .enumerate = enumerate,
            .set_property = set_property,
            .remove_property = remove_property,
            .add = component_ops::add<mesh_renderer_properties>,
            .remove = component_ops::remove<mesh_renderer_properties>,
            .copy = component_ops::copy<mesh_renderer_properties>,
        };
    }
}
