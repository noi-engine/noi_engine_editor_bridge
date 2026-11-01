// Component inspection & editing. Each call looks up the component type's handler (one file
// per type under components/) and forwards to it.

#include "bridge_game.hpp"
#include "components/component_registry.hpp"

namespace noi_engine_editor_bridge_detail
{
    auto bridge_game::component_context_for(const uint32_t entity_id, const uint32_t entity_generation)
        -> component_context
    {
        return {this->get_world(), this->resources(), noi_engine::entity{entity_id, entity_generation}};
    }

    auto bridge_game::enumerate_component_properties(const uint32_t entity_id, const uint32_t entity_generation,
                                                       const std::string& component_type,
                                                       const noi_engine_editor_bridge_property_entry_callback callback,
                                                       void* const user_data) -> void
    {
        if (!callback || !this->get_current_scene())
        {
            return;
        }

        const auto context = component_context_for(entity_id, entity_generation);
        const auto* handler = find_component_handler(component_type);

        if (handler && handler->enumerate && handler->has(context.world, context.entity))
        {
            handler->enumerate(context, property_emitter{callback, user_data});
        }
    }

    auto bridge_game::set_component_property(const uint32_t entity_id, const uint32_t entity_generation,
                                              const std::string& component_type,
                                              const std::string& property_name,
                                              const noi_engine_editor_bridge_property_value& value) -> bool
    {
        if (!this->get_current_scene())
        {
            return false;
        }

        const auto context = component_context_for(entity_id, entity_generation);
        const auto* handler = find_component_handler(component_type);

        return handler && handler->set_property && handler->has(context.world, context.entity) &&
            handler->set_property(context, property_name, value);
    }

    auto bridge_game::remove_component_property(const uint32_t entity_id, const uint32_t entity_generation,
                                                  const std::string& component_type,
                                                  const std::string& property_name) -> bool
    {
        if (!this->get_current_scene())
        {
            return false;
        }

        const auto context = component_context_for(entity_id, entity_generation);
        const auto* handler = find_component_handler(component_type);

        return handler && handler->remove_property && handler->has(context.world, context.entity) &&
            handler->remove_property(context, property_name);
    }

    auto bridge_game::add_component(const uint32_t entity_id, const uint32_t entity_generation,
                                     const std::string& component_type) -> bool
    {
        if (!this->get_current_scene())
        {
            return false;
        }

        const auto* handler = find_component_handler(component_type);
        return handler && handler->add && handler->add(component_context_for(entity_id, entity_generation));
    }

    auto bridge_game::remove_component(const uint32_t entity_id, const uint32_t entity_generation,
                                        const std::string& component_type) -> bool
    {
        if (!this->get_current_scene())
        {
            return false;
        }

        const auto* handler = find_component_handler(component_type);
        return handler && handler->remove && handler->remove(component_context_for(entity_id, entity_generation));
    }
}
