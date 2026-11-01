#pragma once

#include "noi_engine_editor_bridge/bridge_api.h"
#include "properties/property_emitter.hpp"

#include <string>

#include <nlohmann/json.hpp>
#include <noi_engine/core/ecs/entity.hpp>
#include <noi_engine/core/ecs/world.hpp>
#include <noi_engine/core/resources/resources.hpp>

namespace noi_engine_editor_bridge_detail
{
    // The entity a component operation applies to.
    struct component_context
    {
        noi_engine::world& world;
        noi_engine::resources& resources;
        noi_engine::entity entity;
    };

    // Everything the editor can do with one component type. Each type lives in its own file
    // under components/. A null function means the operation isn't supported for that type.
    // enumerate / set_property / remove_property / save are only called when the entity has
    // the component.
    struct component_handler
    {
        using has_fn = bool (*)(noi_engine::world&, noi_engine::entity);
        using enumerate_fn = void (*)(const component_context&, const property_emitter&);
        using set_property_fn = bool (*)(const component_context&, const std::string& property_name,
                                         const noi_engine_editor_bridge_property_value& value);
        using remove_property_fn = bool (*)(const component_context&, const std::string& property_name);
        using add_fn = bool (*)(const component_context&);
        using remove_fn = bool (*)(const component_context&);
        using copy_fn = void (*)(noi_engine::world&, noi_engine::entity from, noi_engine::entity to);
        using save_fn = void (*)(const component_context&, bool is_camera, nlohmann::json& components);

        const char* type{};
        has_fn has{};
        enumerate_fn enumerate{};
        set_property_fn set_property{};
        remove_property_fn remove_property{};
        add_fn add{};       // add with default values; false if already present
        remove_fn remove{}; // false if absent
        copy_fn copy{};     // used by duplicate_entity
        save_fn save{};     // appends the component's scene JSON (see bridge_game_persistence.cpp)
    };

    // Default implementations shared by most component types.
    namespace component_ops
    {
        template <typename T>
        auto has(noi_engine::world& world, const noi_engine::entity e) -> bool
        {
            return world.has<T>(e);
        }

        template <typename T>
        auto add(const component_context& c) -> bool
        {
            if (c.world.has<T>(c.entity))
            {
                return false;
            }
            c.world.add<T>(c.entity, {});
            return true;
        }

        template <typename T>
        auto remove(const component_context& c) -> bool
        {
            if (!c.world.has<T>(c.entity))
            {
                return false;
            }
            c.world.remove<T>(c.entity);
            return true;
        }

        template <typename T>
        auto copy(noi_engine::world& world, const noi_engine::entity from, const noi_engine::entity to) -> void
        {
            if (world.has<T>(from))
            {
                world.add<T>(to, world.get<T>(from));
            }
        }
    }
}
