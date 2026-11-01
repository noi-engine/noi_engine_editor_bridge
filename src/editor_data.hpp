#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <noi_engine/core/components/camera_2d.hpp>
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

        // Hierarchy folder of each entity ("Level/Platforms"), read from / written to the
        // entities' "folder" field in the scene file. Never part of the game.
        std::unordered_map<uint64_t, std::string> folders{};

        [[nodiscard]] static auto key(const noi_engine::entity e) -> uint64_t
        {
            return (static_cast<uint64_t>(e.m_id) << 32) | e.m_generation;
        }
    };
}
