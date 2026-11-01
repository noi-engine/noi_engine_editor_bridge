#include "component_handler.hpp"
#include "properties/property_value.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/script_component.hpp>
#include <noi_engine/core/model/script.hpp>

// Scripts attached to an entity, exposed as "scripts[<index>]" resource references:
//   - set "scripts[i]" to a script name: replaces that script; "scripts[<count>]" attaches a new one.
//   - remove_property "scripts[i]": detaches it.
// Any script of the scripts library can be attached (loaded on demand).
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::script_component;

        // "scripts[3]" -> 3. False for any other name.
        auto parse_index(const std::string& name, size_t& index) -> bool
        {
            constexpr std::string_view prefix = "scripts[";
            if (!name.starts_with(prefix) || name.size() <= prefix.size() + 1 || name.back() != ']')
            {
                return false;
            }
            const auto digits = name.substr(prefix.size(), name.size() - prefix.size() - 1);
            if (digits.find_first_not_of("0123456789") != std::string::npos)
            {
                return false;
            }
            index = static_cast<size_t>(std::stoul(digits));
            return true;
        }

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& sc = c.world.get<script_component>(c.entity);

            for (size_t i = 0; i < sc.scripts.size(); ++i)
            {
                const std::string label{c.resources.get_label<noi_engine::script>(sc.scripts[i])};
                const std::string prop_name = "scripts[" + std::to_string(i) + "]";
                emitter.emit_resource_ref(prop_name.c_str(), label, NOI_ENGINE_EDITOR_BRIDGE_RESOURCE_SCRIPT, false);
            }
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            size_t index = 0;
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_RESOURCE_REF || !parse_index(name, index))
            {
                return false;
            }

            auto& sc = c.world.get<script_component>(c.entity);
            if (index > sc.scripts.size())
            {
                return false;
            }

            noi_engine::resource_handle<noi_engine::script> handle{};
            try
            {
                if (!c.resources.find_or_load_script(text_of(value), handle))
                {
                    return false;
                }
            }
            catch (const std::exception&)
            {
                return false;
            }

            if (index == sc.scripts.size())
            {
                sc.scripts.push_back(handle);
            }
            else
            {
                sc.scripts[index] = handle;
            }
            return true;
        }

        auto remove_property(const component_context& c, const std::string& name) -> bool
        {
            size_t index = 0;
            auto& sc = c.world.get<script_component>(c.entity);
            if (!parse_index(name, index) || index >= sc.scripts.size())
            {
                return false;
            }
            sc.scripts.erase(sc.scripts.begin() + static_cast<std::ptrdiff_t>(index));
            return true;
        }

        // The copy's scripts haven't been created yet (on_create runs for it too).
        auto copy(const component_context& from, const noi_engine::entity to) -> void
        {
            auto value = from.world.get<script_component>(from.entity);
            value.created = false;
            from.world.add<script_component>(to, value);
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            auto names = nlohmann::json::array();
            for (const auto& handle : c.world.get<script_component>(c.entity).scripts)
            {
                if (const auto label = c.resources.get_label<noi_engine::script>(handle); !label.empty())
                {
                    names.push_back(std::string(label));
                }
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
            .remove_property = remove_property,
            .add = component_ops::add<script_component>,
            .remove = component_ops::remove<script_component>,
            .copy = copy,
            .save = save,
        };
    }
}
