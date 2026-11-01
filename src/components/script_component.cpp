#include "component_handler.hpp"
#include "properties/property_value.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/script_component.hpp>
#include <noi_engine/core/model/script.hpp>

// Scripts are exposed as "scripts[<index>]" resource references.
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::script_component;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& sc = c.world.get<script_component>(c.entity);

            for (size_t i = 0; i < sc.scripts.size(); ++i)
            {
                const std::string label{c.resources.get_label<noi_engine::script>(sc.scripts[i])};
                const std::string prop_name = "scripts[" + std::to_string(i) + "]";
                emitter.emit_resource_ref(prop_name.c_str(), label, NOI_ENGINE_EDITOR_BRIDGE_RESOURCE_SCRIPT, false);
            }
            emitter.emit_bool("created", sc.created, true);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_RESOURCE_REF ||
                !name.starts_with("scripts[") || name.back() != ']')
            {
                return false;
            }

            auto& sc = c.world.get<script_component>(c.entity);
            const auto index = static_cast<size_t>(std::stoul(name.substr(8, name.size() - 9)));

            if (index >= sc.scripts.size())
            {
                return false;
            }
            sc.scripts[index] = c.resources.get_handle<noi_engine::script>(text_of(value));
            return true;
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            auto names = nlohmann::json::array();
            for (const auto& handle : c.world.get<script_component>(c.entity).scripts)
            {
                names.push_back(std::string(c.resources.get_label<noi_engine::script>(handle)));
            }
            components.push_back({{"type", "scripts"}, {"properties", {{"names", names}}}});
        }
    }

    auto make_script_component_handler() -> component_handler
    {
        return {
            .type = "script_component",
            .has = component_ops::has<script_component>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<script_component>,
            .remove = component_ops::remove<script_component>,
            .copy = component_ops::copy<script_component>,
            .save = save,
        };
    }
}
