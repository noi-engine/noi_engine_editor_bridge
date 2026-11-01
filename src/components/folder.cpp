#include "component_handler.hpp"
#include "properties/property_value.hpp"

// The entity's Hierarchy folder, exposed as a "folder" component with a "path" property so the
// editor can use the regular component calls. Editor-only: stored in editor_data, not in the
// world, and written as the entity's "folder" field by save_scene().
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        auto has(const component_context& c) -> bool
        {
            return c.editor.folders.contains(editor_data::key(c.entity));
        }

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            emitter.emit_string("path", c.editor.folders.at(editor_data::key(c.entity)), false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (name != "path" || value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_STRING)
            {
                return false;
            }
            c.editor.folders[editor_data::key(c.entity)] = text_of(value);
            return true;
        }

        auto add(const component_context& c) -> bool
        {
            return c.editor.folders.try_emplace(editor_data::key(c.entity)).second;
        }

        auto remove(const component_context& c) -> bool
        {
            return c.editor.folders.erase(editor_data::key(c.entity)) > 0;
        }

        auto copy(const component_context& from, const noi_engine::entity to) -> void
        {
            if (const auto it = from.editor.folders.find(editor_data::key(from.entity)); it != from.editor.folders.end())
            {
                auto path = it->second; // copy first: inserting below may rehash and invalidate `it`
                from.editor.folders[editor_data::key(to)] = std::move(path);
            }
        }
    }

    auto make_folder_handler() -> component_handler
    {
        return {
            .type = "folder",
            .has = has,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = add,
            .remove = remove,
            .copy = copy,
        };
    }
}
