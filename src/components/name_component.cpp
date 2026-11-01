#include "component_handler.hpp"
#include "properties/property_value.hpp"

#include <noi_engine/core/components/name_component.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::name_component;

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            emitter.emit_string("name", c.world.get<name_component>(c.entity).name, false);
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (name != "name" || value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_STRING)
            {
                return false;
            }
            c.world.get<name_component>(c.entity).name = text_of(value);
            return true;
        }
    }

    // Not removable: it is the entity's identity and the Hierarchy relies on every entity
    // having one. Not copied by duplicate_entity either (the copy gets its own name).
    auto make_name_component_handler() -> component_handler
    {
        return {
            .type = "name_component",
            .has = component_ops::has<name_component>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<name_component>,
        };
    }
}
