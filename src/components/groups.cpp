#include "component_handler.hpp"
#include "properties/property_value.hpp"

// The entity's Hierarchy folder, exposed as a "groups" component with a "names" property so the
// editor can use the regular component calls. Editor-only: stored in editor_data, not in the
// world, and written as the entity's "groups" field by save_scene().
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        auto has(const component_context& c) -> bool
        {
            return c.editor.groups.contains(editor_data::key(c.entity));
        }

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            emitter.emit_string("names", c.editor.groups.at(editor_data::key(c.entity)), false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (name != "names" || value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_STRING)
            {
                return false;
            }
            c.editor.groups[editor_data::key(c.entity)] = text_of(value);
            return true;
        }

        auto add(const component_context& c) -> bool
        {
            return c.editor.groups.try_emplace(editor_data::key(c.entity)).second;
        }

        auto remove(const component_context& c) -> bool
        {
            return c.editor.groups.erase(editor_data::key(c.entity)) > 0;
        }

        auto copy(const component_context& from, const noi_engine::entity to) -> void
        {
            if (const auto it = from.editor.groups.find(editor_data::key(from.entity)); it != from.editor.groups.end())
            {
                auto names = it->second; // copy first: inserting below may rehash and invalidate `it`
                from.editor.groups[editor_data::key(to)] = std::move(names);
            }
        }
    }

    auto make_groups_handler() -> component_handler
    {
        return {
            .type = "groups",
            .has = has,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add,
            .remove = remove,
            .copy = copy,
        };
    }
}
