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

    auto bridge_game::is_editor_entity(const noi_engine::entity e) const -> bool
    {
        return m_has_gizmo_entity && e == m_gizmo_entity;
    }

    // The engine ignores the entities' editor-only "folder" and "groups" fields, so read them
    // straight from the scene file.
    // Entities are matched by name, in file order (the order the loader created them in).
    auto bridge_game::load_editor_fields(const std::string& scene_path) -> void
    {
        m_editor.folders.clear();
        m_editor.groups.clear();

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

        // Per name, in file order: {folder, groups}.
        std::unordered_map<std::string, std::deque<std::pair<std::string, std::string>>> fields_by_name;
        for (const auto& entity : scene["entities"])
        {
            std::string groups;
            for (const auto& group : entity.value("groups", nlohmann::json::array()))
            {
                if (group.is_string()) groups += (groups.empty() ? "" : ",") + group.get<std::string>();
            }
            fields_by_name[entity.value("name", "")].emplace_back(entity.value("folder", ""), groups);
        }

        auto& world = this->get_world();
        for (const auto& e : world.get_entities())
        {
            if (!world.has<noi_engine::name_component>(e))
            {
                continue;
            }
            auto& queue = fields_by_name[world.get<noi_engine::name_component>(e).name];
            if (queue.empty())
            {
                continue;
            }
            const auto& [folder, groups] = queue.front();
            if (!folder.empty())
            {
                m_editor.folders[editor_data::key(e)] = folder;
            }
            if (!groups.empty())
            {
                m_editor.groups[editor_data::key(e)] = groups;
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
            // The camera border (see set_scene_2d()) is a rendering aid, not scene content.
            if (is_editor_entity(e))
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

        // The scene's camera and the editor's camera border are structural, not authored content.
        if (e == this->get_current_scene()->get_camera_entity() || is_editor_entity(e))
        {
            return false;
        }
        if (std::ranges::find(world.get_entities(), e) == world.get_entities().end())
        {
            return false;
        }

        world.destroy_entity(e);
        m_editor.folders.erase(editor_data::key(e));
        m_editor.groups.erase(editor_data::key(e));
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
