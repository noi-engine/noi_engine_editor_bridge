// Scene loading plus the editor-only visuals: a red border showing the area the scene camera
// films in the game. Outside Play mode the live camera follows the editor's own view (pan /
// zoom), the scene camera keeps its authored position and the border sits there.

#include "bridge_game.hpp"
#include "editor_shaders.hpp"

#include <cstdio>
#include <exception>
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
        // Drawn above the scene.
        constexpr int GIZMO_RENDER_ORDER = 1000;
    }

    auto bridge_game::set_scene_2d(const std::string& path) -> void
    {
        // A broken scene file must not take the editor down with it.
        try
        {
            this->set_scene(path);
        }
        catch (const std::exception& e)
        {
            std::fprintf(stderr, "[noi_engine_editor_bridge] Couldn't load the scene %s: %s\n", path.c_str(), e.what());
            return;
        }
        if (!this->get_current_scene())
        {
            return;
        }
        this->get_current_scene()->add_system<noi_engine::physics_system>();
        this->get_current_scene()->set_scripts_paused(!m_playing);
        load_registered_scripts();

        // Remember the camera as authored, before on_resize() / the editor view change it.
        m_editor.camera_entity = this->get_current_scene()->get_camera_entity();
        m_editor.authored_camera = this->get_camera_2d_component();
        m_editor.authored_camera_transform = this->get_world().get<noi_engine::transform>(m_editor.camera_entity);

        load_editor_fields(path);
        add_camera_gizmo();
        apply_camera_mode();
    }

    auto bridge_game::set_editor_view(const float x, const float y, const float zoom) -> void
    {
        m_editor.view_x = x;
        m_editor.view_y = y;
        m_editor.view_zoom = zoom > 0.0f ? zoom : 1.0f;
        if (m_playing)
        {
            return; // in Play mode the game (its scripts) drives the camera
        }
        apply_camera_mode();
        refresh_camera_gizmo(); // its border keeps the same on-screen width at any zoom
    }

    // Edit mode: the live camera shows the editor's view. Play mode: it starts where the scene
    // camera is (scripts then move it).
    auto bridge_game::apply_camera_mode() -> void
    {
        if (!this->get_current_scene())
        {
            return;
        }

        auto& world = this->get_world();
        auto& transform = world.get<noi_engine::transform>(m_editor.camera_entity);
        auto& camera = this->get_camera_2d_component();

        if (m_playing)
        {
            transform.position = m_editor.authored_camera_transform.position;
            camera.zoom = m_editor.authored_camera.zoom;
        }
        else
        {
            transform.position = glm::vec3{m_editor.view_x, m_editor.view_y, m_editor.authored_camera_transform.position.z};
            camera.zoom = m_editor.view_zoom;
        }
        world.add<noi_engine::dirty<noi_engine::transform>>(m_editor.camera_entity, noi_engine::dirty<noi_engine::transform>{});
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

        auto& world = this->get_world();
        m_gizmo_entity = world.create_entity();
        m_has_gizmo_entity = true;
        world.add_transform(m_gizmo_entity, {});
        world.add<noi_engine::render_layer>(m_gizmo_entity, {.order = GIZMO_RENDER_ORDER});
        world.add<noi_engine::mesh_renderer>(m_gizmo_entity, {
                                                 .m_mesh = resources_ref.get_handle<noi_engine::mesh>("meshes:quad_2d"),
                                                 .m_material = m_gizmo_material
                                             });
        refresh_camera_gizmo();
    }

    // Puts the red border where the scene camera is, sized to the area it films.
    auto bridge_game::refresh_camera_gizmo() -> void
    {
        if (!m_has_gizmo_entity)
        {
            return;
        }

        const auto& camera = m_editor.authored_camera;
        const float world_height = camera.orthographic_size * 2.0f / camera.zoom;
        const float world_width = world_height * camera.aspect_ratio;

        if (auto* material = this->resources().materials.get(m_gizmo_material))
        {
            const float border = GIZMO_BORDER_PIXELS / PIXELS_PER_UNIT / m_editor.view_zoom;
            material->set_parameter("u_border_thickness_x", border / world_width);
            material->set_parameter("u_border_thickness_y", border / world_height);
        }

        auto& world = this->get_world();
        auto& transform = world.get<noi_engine::transform>(m_gizmo_entity);
        // In Play mode the border follows the live camera (moved by the game's scripts).
        transform.position = m_playing ? world.get<noi_engine::transform>(m_editor.camera_entity).position
                                       : m_editor.authored_camera_transform.position;
        transform.scale = glm::vec3{world_width, world_height, 1.0f};
        world.add<noi_engine::dirty<noi_engine::transform>>(m_gizmo_entity, noi_engine::dirty<noi_engine::transform>{});
    }

    // The world grid was removed; kept so older editors that still call it keep working.
    auto bridge_game::set_grid_visible(bool) -> void
    {
    }
}
