#pragma once

#include "noi_engine_editor_bridge/bridge_api.h"
#include "components/component_handler.hpp"
#include "editor_data.hpp"

#include <cstdint>
#include <string>
#include <unordered_set>

#include <noi_engine/core/components/camera_2d.hpp>
#include <noi_engine/core/ecs/entity.hpp>
#include <noi_engine/core/ecs/world.hpp>
#include <noi_engine/core/game.hpp>
#include <noi_engine/core/timer.hpp>

namespace noi_engine_editor_bridge_detail
{
    // Concrete noi_engine::game behind every noi_engine_editor_bridge_game_handle. bridge_api.cpp
    // exposes it through the extern "C" API in bridge_api.h. Implementation files:
    //   - bridge_game.cpp:             lifecycle, input, resize, scripts library
    //   - bridge_game_overlay.cpp:     scene loading, editor view, editor-only camera border
    //   - bridge_game_entities.cpp:    entity listing, create/destroy/duplicate
    //   - bridge_game_components.cpp:  component editing (one handler per type in components/)
    //   - bridge_game_resources.cpp:   resource listing and material editing
    //   - bridge_game_persistence.cpp: save the scene back to JSON
    class bridge_game final : public noi_engine::game
    {
    public:
        bridge_game(int game_width, int game_height, const std::string& resources_path);
        ~bridge_game() override;

        // Lifecycle & input
        auto run() -> void;
        auto reset_timer() -> void;
        auto set_playing(bool playing) -> void;
        auto set_key_pressed(int key) -> void;
        auto set_key_released(int key) -> void;
        auto release_all_keys() -> void;
        auto on_resize(int width, int height) -> void;
        auto load_scripts(const std::string& scripts_library_path) -> bool;

        // Scene & editor overlay
        auto set_scene_2d(const std::string& path) -> void;
        auto set_grid_visible(bool visible) -> void; // no-op, the grid was removed
        auto set_editor_view(float x, float y, float zoom) -> void;
        [[nodiscard]] auto save_scene(const std::string& target_path) -> bool;

        // Entities
        auto enumerate_entities(noi_engine_editor_bridge_entity_entry_callback callback, void* user_data) -> void;
        [[nodiscard]] auto create_entity(const std::string& name) -> noi_engine::entity;
        [[nodiscard]] auto destroy_entity(uint32_t entity_id, uint32_t entity_generation) -> bool;
        [[nodiscard]] auto duplicate_entity(uint32_t entity_id, uint32_t entity_generation,
                                             const std::string& new_name) -> noi_engine::entity;

        // Components
        auto enumerate_component_properties(uint32_t entity_id, uint32_t entity_generation,
                                             const std::string& component_type,
                                             noi_engine_editor_bridge_property_entry_callback callback,
                                             void* user_data) -> void;
        [[nodiscard]] auto set_component_property(uint32_t entity_id, uint32_t entity_generation,
                                                   const std::string& component_type,
                                                   const std::string& property_name,
                                                   const noi_engine_editor_bridge_property_value& value) -> bool;
        [[nodiscard]] auto remove_component_property(uint32_t entity_id, uint32_t entity_generation,
                                                       const std::string& component_type,
                                                       const std::string& property_name) -> bool;
        [[nodiscard]] auto add_component(uint32_t entity_id, uint32_t entity_generation,
                                          const std::string& component_type) -> bool;
        [[nodiscard]] auto remove_component(uint32_t entity_id, uint32_t entity_generation,
                                             const std::string& component_type) -> bool;

        // Resources & materials
        auto enumerate_resources(noi_engine_editor_bridge_resource_entry_callback callback,
                                  void* user_data) const -> void;
        auto enumerate_material_properties(uint32_t material_id, uint32_t material_generation,
                                            noi_engine_editor_bridge_property_entry_callback callback,
                                            void* user_data) -> void;
        [[nodiscard]] auto set_material_property(uint32_t material_id, uint32_t material_generation,
                                                  const std::string& property_name,
                                                  const noi_engine_editor_bridge_property_value& value) -> bool;
        [[nodiscard]] auto remove_material_property(uint32_t material_id, uint32_t material_generation,
                                                      const std::string& property_name) -> bool;

    private:
        static constexpr float PIXELS_PER_UNIT = 100.0f;

        [[nodiscard]] auto get_world() -> noi_engine::world&;
        [[nodiscard]] auto get_current_scene() const -> noi_engine::scene*;

        //TODO: handle 3d
        [[nodiscard]] auto get_camera_2d_component() -> noi_engine::camera_2d&;

        [[nodiscard]] auto component_context_for(noi_engine::entity e) -> component_context;
        [[nodiscard]] auto component_context_for(uint32_t entity_id, uint32_t entity_generation) -> component_context;
        // Entities the editor adds for display (the camera border): never listed, saved or deletable.
        [[nodiscard]] auto is_editor_entity(noi_engine::entity e) const -> bool;

        auto add_camera_gizmo() -> void;
        auto refresh_camera_gizmo() -> void;
        auto apply_camera_mode() -> void;
        auto load_editor_fields(const std::string& scene_path) -> void;
        auto load_registered_scripts() -> void;

        std::string m_resources_path;
        editor_data m_editor{};
        noi_engine::resource_handle<noi_engine::material> m_gizmo_material{};
        noi_engine::timer m_timer{};
        void* m_scripts_library_handle{nullptr};
        unsigned m_scripts_load_count{0};
        bool m_playing{false};
        std::unordered_set<int> m_held_keys{};

        noi_engine::entity m_gizmo_entity{};
        bool m_has_gizmo_entity{false};
    };
}
