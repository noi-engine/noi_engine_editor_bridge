// Scene loading plus the editor-only visuals added on top of every scene: a red border
// showing the game camera's bounds, and an optional world grid.

#include "bridge_game.hpp"
#include "editor_shaders.hpp"

#include <memory>

#include <glm/glm.hpp>

#include <noi_engine/builtin/systems/physics_system.hpp>
#include <noi_engine/core/components/dirty.hpp>
#include <noi_engine/core/components/mesh_renderer.hpp>
#include <noi_engine/core/components/render_layer.hpp>
#include <noi_engine/core/components/transform.hpp>
#include <noi_engine/core/model/material.hpp>
#include <noi_engine/core/model/shader.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        constexpr float GIZMO_BORDER_PIXELS = 2.0f;

        // Grid quad size in world units - comfortably larger than any zoom level the editor allows.
        constexpr float GRID_QUAD_SIZE = 200.0f;
        constexpr float GRID_CELL_SIZE = 1.0f;
        constexpr float GRID_LINE_THICKNESS = 0.006f;
    }

    auto bridge_game::set_scene_2d(const std::string& path) -> void
    {
        this->set_scene(path);
        this->get_current_scene()->add_system<noi_engine::physics_system>();

        // Remember the camera as authored, before on_resize() stretches it to the viewport.
        m_editor.camera_entity = this->get_current_scene()->get_camera_entity();
        m_editor.authored_camera = this->get_camera_2d_component();

        load_folders(path);
        add_camera_gizmo();
        create_grid();
    }

    auto bridge_game::add_camera_gizmo() -> void
    {
        auto& resources_ref = this->resources();

        const auto shader_handle = resources_ref.load_instance(
            "camera_gizmo_shader",
            std::make_unique<noi_engine::shader>(editor_shaders::QUAD_VERTEX, editor_shaders::CAMERA_GIZMO_FRAGMENT));

        auto material = std::make_unique<noi_engine::material>();
        material->set_color("u_color", glm::vec4{1, 0, 0, 1});
        material->set_shader(shader_handle);
        m_gizmo_material = resources_ref.load_instance(std::move(material));

        this->get_world().add<noi_engine::mesh_renderer>(m_editor.camera_entity, {
                                                             .m_mesh = resources_ref.get_handle<noi_engine::mesh>(
                                                                 "meshes:quad_2d"),
                                                             .m_material = m_gizmo_material
                                                         });
        refresh_camera_gizmo();
    }

    // Sizes the red border to the area the authored camera shows in the game.
    auto bridge_game::refresh_camera_gizmo() -> void
    {
        const auto& camera = m_editor.authored_camera;
        const float world_height = camera.orthographic_size * 2.0f / camera.zoom;
        const float world_width = world_height * camera.aspect_ratio;

        if (auto* material = this->resources().materials.get(m_gizmo_material))
        {
            const float border = GIZMO_BORDER_PIXELS / PIXELS_PER_UNIT;
            material->set_parameter("u_border_thickness_x", border / world_width);
            material->set_parameter("u_border_thickness_y", border / world_height);
        }

        auto& world = this->get_world();
        world.get<noi_engine::transform>(m_editor.camera_entity).scale = glm::vec3{world_width, world_height, 1.0f};
        world.add<noi_engine::dirty<noi_engine::transform>>(m_editor.camera_entity,
                                                            noi_engine::dirty<noi_engine::transform>{});
    }

    // Fixed in world space (its own entity, not attached to the camera) so panning/following
    // the camera visibly scrolls it underneath. Hidden by default (see m_grid_visible) and
    // toggled from the editor's viewport toolbar.
    auto bridge_game::create_grid() -> void
    {
        auto& resources_ref = this->resources();

        const auto grid_shader_handle = resources_ref.load_instance(
            "world_grid_shader",
            std::make_unique<noi_engine::shader>(editor_shaders::QUAD_VERTEX, editor_shaders::WORLD_GRID_FRAGMENT));

        auto grid_material = std::make_unique<noi_engine::material>();
        grid_material->set_color("u_line_color", glm::vec4{1, 1, 1, 0.2f});
        grid_material->set_parameter("u_cell_size", GRID_CELL_SIZE);
        grid_material->set_parameter("u_line_thickness", GRID_LINE_THICKNESS);
        grid_material->set_parameter("u_quad_size", GRID_QUAD_SIZE);
        grid_material->set_shader(grid_shader_handle);

        auto& world = this->get_world();
        const auto grid_entity = world.create_entity();
        world.add_transform(grid_entity, noi_engine::transform{
                                 .scale = glm::vec3{GRID_QUAD_SIZE, GRID_QUAD_SIZE, 1.0f}
                             });
        world.add<noi_engine::render_layer>(grid_entity, {.order = -10});

        m_grid_entity = grid_entity;
        m_has_grid_entity = true;
        m_grid_mesh = resources_ref.get_handle<noi_engine::mesh>("meshes:quad_2d");
        m_grid_material = resources_ref.load_instance(std::move(grid_material));

        if (m_grid_visible)
        {
            world.add<noi_engine::mesh_renderer>(grid_entity, {.m_mesh = m_grid_mesh, .m_material = m_grid_material});
        }
    }

    auto bridge_game::set_grid_visible(const bool visible) -> void
    {
        m_grid_visible = visible;

        if (!this->get_current_scene() || !m_has_grid_entity)
        {
            return;
        }

        auto& world = this->get_world();
        const bool has_renderer = world.has<noi_engine::mesh_renderer>(m_grid_entity);

        if (visible && !has_renderer)
        {
            world.add<noi_engine::mesh_renderer>(m_grid_entity, {.m_mesh = m_grid_mesh, .m_material = m_grid_material});
        }
        else if (!visible && has_renderer)
        {
            world.remove<noi_engine::mesh_renderer>(m_grid_entity);
        }
    }
}
