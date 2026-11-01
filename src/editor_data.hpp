#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <noi_engine/core/components/camera_2d.hpp>
#include <noi_engine/core/components/transform.hpp>
#include <noi_engine/core/ecs/entity.hpp>

namespace noi_engine_editor_bridge_detail
{
    // Editor-only state the engine doesn't know about, kept by the bridge next to the world.
    struct editor_data
    {
        // The scene's camera entity and its camera_2d as authored in the scene. The live
        // camera_2d is stretched to the editor viewport (see bridge_game::on_resize), so the
        // Inspector, the red camera border and save_scene() use this copy instead.
        noi_engine::entity camera_entity{};
        noi_engine::camera_2d authored_camera{};
        // Same for its transform: outside Play mode the live camera follows the editor's view
        // (set_editor_view), the scene camera's position stays here.
        noi_engine::transform authored_camera_transform{};

        // Where the editor looks (world units) and how zoomed in, outside Play mode.
        float view_x{0.0f};
        float view_y{0.0f};
        float view_zoom{1.0f};

        // Hierarchy folder of each entity ("Level/Platforms"), read from / written to the
        // entities' "folder" field in the scene file. Never part of the game.
        std::unordered_map<uint64_t, std::string> folders{};

        // Editor groups of each entity, comma separated ("sprite"): components the editor added
        // together and shows as one section. Read from / written to the entities' "groups"
        // field. Never part of the game.
        std::unordered_map<uint64_t, std::string> groups{};

        [[nodiscard]] static auto key(const noi_engine::entity e) -> uint64_t
        {
            return (static_cast<uint64_t>(e.m_id) << 32) | e.m_generation;
        }
    };
}
