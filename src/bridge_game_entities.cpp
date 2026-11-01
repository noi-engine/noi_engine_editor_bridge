#include "bridge_game.hpp"
#include "components/component_registry.hpp"

#include <algorithm>
#include <deque>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

#include <noi_engine/core/components/name_component.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        auto add_name(noi_engine::world& world, const noi_engine::entity e, const std::string& name) -> void
        {
            noi_engine::name_component nc{};
            nc.name = name;
            world.add<noi_engine::name_component>(e, nc);
        }
    }

    auto bridge_game::is_grid_entity(const noi_engine::entity e) const -> bool
    {
        return m_has_grid_entity && e == m_grid_entity;
    }

    // The engine ignores the entities' "folder" field, so read it straight from the scene file.
    // Entities are matched by name, in file order (the order the loader created them in).
    auto bridge_game::load_folders(const std::string& scene_path) -> void
    {
        m_editor.folders.clear();

        std::ifstream in(std::filesystem::path(m_resources_path) / scene_path);
        if (!in)
        {
            return;
        }
        const auto scene = nlohmann::json::parse(in, nullptr, false);
        if (!scene.is_object() || !scene.contains("entities"))
        {
            return;
        }

        std::unordered_map<std::string, std::deque<std::string>> folders_by_name;
        for (const auto& entity : scene["entities"])
        {
            folders_by_name[entity.value("name", "")].push_back(entity.value("folder", ""));
        }

        auto& world = this->get_world();
        for (const auto& e : world.get_entities())
        {
            if (!world.has<noi_engine::name_component>(e))
            {
                continue;
            }
            auto& queue = folders_by_name[world.get<noi_engine::name_component>(e).name];
            if (queue.empty())
            {
                continue;
            }
            if (!queue.front().empty())
            {
                m_editor.folders[editor_data::key(e)] = queue.front();
            }
            queue.pop_front();
        }
    }

    auto bridge_game::enumerate_entities(const noi_engine_editor_bridge_entity_entry_callback callback,
                                          void* const user_data) -> void
    {
        if (!callback || !this->get_current_scene())
        {
            return;
        }

        auto& world = this->get_world();

        for (const auto& e : world.get_entities())
        {
            // The editor's reference grid (see set_scene_2d()) is a rendering aid, not scene
            // content - keep it out of the Hierarchy.
            if (is_grid_entity(e))
            {
                continue;
            }

            std::string name;
            if (world.has<noi_engine::name_component>(e))
            {
                name = world.get<noi_engine::name_component>(e).name;
            }

            std::string component_types;
            const auto context = component_context_for(e);
            for (const auto& handler : component_handlers())
            {
                if (handler.has(context))
                {
                    component_types += (component_types.empty() ? "" : ",") + std::string(handler.type);
                }
            }

            callback(user_data, e.m_id, e.m_generation, name.c_str(), component_types.c_str());
        }
    }

    auto bridge_game::create_entity(const std::string& name) -> noi_engine::entity
    {
        if (!this->get_current_scene())
        {
            return {};
        }

        auto& world = this->get_world();
        const auto e = world.create_entity();
        add_name(world, e, name);
        return e;
    }

    auto bridge_game::destroy_entity(const uint32_t entity_id, const uint32_t entity_generation) -> bool
    {
        if (!this->get_current_scene())
        {
            return false;
        }

        auto& world = this->get_world();
        const noi_engine::entity e{entity_id, entity_generation};

        // The scene's camera and the editor's grid are structural, not authored content.
        if (e == this->get_current_scene()->get_camera_entity() || is_grid_entity(e))
        {
            return false;
        }
        if (std::ranges::find(world.get_entities(), e) == world.get_entities().end())
        {
            return false;
        }

        world.destroy_entity(e);
        m_editor.folders.erase(editor_data::key(e));
        return true;
    }

    auto bridge_game::duplicate_entity(const uint32_t entity_id, const uint32_t entity_generation,
                                        const std::string& new_name) -> noi_engine::entity
    {
        if (!this->get_current_scene())
        {
            return {};
        }

        auto& world = this->get_world();
        const noi_engine::entity e{entity_id, entity_generation};

        if (std::ranges::find(world.get_entities(), e) == world.get_entities().end())
        {
            return {};
        }

        // Copies every copyable component (all but name_component and world_matrix), then
        // gives the copy its new name.
        const auto copy = world.create_entity();
        const auto source = component_context_for(e);
        for (const auto& handler : component_handlers())
        {
            if (handler.copy)
            {
                handler.copy(source, copy);
            }
        }

        add_name(world, copy, new_name);
        return copy;
    }
}
