// Writes the live world back to source-loader-compatible scene JSON. Each component type
// writes its own JSON (the `save` function in components/<type>.cpp); see
// noi_engine/sources/core/resources/source_loader/loader/scene_parser/component_parser.cpp
// for the shape each type must round-trip through.

#include "bridge_game.hpp"
#include "components/component_registry.hpp"

#include <array>
#include <fstream>

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/name_component.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        // Order components are written in. Types without a `save` function aren't persisted
        // (gravity, for instance, has no scene format: scripts add it at runtime).
        constexpr std::array SAVE_ORDER{
            "transform", "camera_2d", "render_layer", "mesh_renderer", "script_component", "box_collider",
        };

        // The engine doesn't keep a resource's load path once loaded (only its label), so the
        // scene's "name" and "resources" blocks are carried over from the file on disk.
        auto read_existing_header(const std::string& path, nlohmann::json& name, nlohmann::json& resources) -> void
        {
            name = "Untitled Scene";
            resources = nlohmann::json::object();

            std::ifstream in(path);
            if (!in)
            {
                return;
            }

            const auto existing = nlohmann::json::parse(in, nullptr, false);
            if (!existing.is_object())
            {
                return;
            }
            if (existing.contains("name")) name = existing["name"];
            if (existing.contains("resources")) resources = existing["resources"];
        }
    }

    auto bridge_game::save_scene(const std::string& target_path) -> bool
    {
        const auto* current_scene = this->get_current_scene();
        if (!current_scene)
        {
            return false;
        }

        auto& world = this->get_world();
        const auto camera_entity = current_scene->get_camera_entity();

        nlohmann::json root;
        read_existing_header(target_path, root["name"], root["resources"]);

        auto entities = nlohmann::json::array();

        for (const auto& e : world.get_entities())
        {
            // The reference grid is editor-only (see set_scene_2d()) - never write it back.
            if (is_grid_entity(e))
            {
                continue;
            }

            const bool is_camera = (e == camera_entity);

            std::string name = is_camera ? "main_camera" : std::string{};
            if (!is_camera && world.has<noi_engine::name_component>(e))
            {
                name = world.get<noi_engine::name_component>(e).name;
            }

            const auto context = component_context_for(e);
            auto components = nlohmann::json::array();

            for (const auto* type : SAVE_ORDER)
            {
                const auto* handler = find_component_handler(type);
                if (handler && handler->save && handler->has(context))
                {
                    handler->save(context, is_camera, components);
                }
            }

            nlohmann::json entity_json{{"name", name}, {"components", components}};
            if (const auto it = m_editor.folders.find(editor_data::key(e)); it != m_editor.folders.end() &&
                !it->second.empty())
            {
                entity_json["folder"] = it->second;
            }
            entities.push_back(entity_json);
        }

        root["entities"] = entities;

        std::ofstream out(target_path);
        if (!out)
        {
            return false;
        }
        out << root.dump(2);
        return out.good();
    }
}
