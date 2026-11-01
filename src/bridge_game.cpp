#include "bridge_game.hpp"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <memory>

#include <dlfcn.h>

#include <noi_engine/core/components/camera_2d.hpp>
#include <noi_engine/core/renderer/renderer.hpp>
#include <noi_engine/core/resources/resource_loader.hpp>

namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        constexpr auto REGISTER_SCRIPTS_SYMBOL = "noi_engine_register_scripts";
        using register_scripts_fn = void (*)(noi_engine::game&);
    }

    // The game size the editor passes is no longer used: the red camera border shows the scene
    // camera's own camera_2d.
    bridge_game::bridge_game(int /*game_width*/, int /*game_height*/, const std::string& resources_path) :
        game(std::make_unique<noi_engine::source_resource_loader>(std::filesystem::path(resources_path))),
        m_resources_path(resources_path)
    {
    }

    bridge_game::~bridge_game()
    {
        // Intentionally not dlclose()-ing m_scripts_library_handle here: this destructor body
        // runs *before* the base noi_engine::game destructor, which tears down the scene/world -
        // including any entities holding instances of script classes whose code lives in this
        // library. Unloading it first leaves their vtables/destructors dangling and crashes.
        // Leaking the mapping for the rest of the process's life (reclaimed at exit) is the
        // safe tradeoff here.
    }

    auto bridge_game::run() -> void
    {
        const float dt = m_timer.tick();

        // Edit mode: zero time step, so nothing moves on its own while the scene is edited.
        this->update(m_playing ? dt : 0.0f);

        noi_engine::renderer::begin_frame();
        render();
        noi_engine::renderer::end_frame();

        // Must run after this frame's update()/render() consumed the current edge state,
        // and before the next frame's key deltas are applied (see set_key_pressed/released) -
        // otherwise is_key_pressed()/is_key_released() could never reset.
        this->input().begin_frame();
    }

    auto bridge_game::set_playing(const bool playing) -> void
    {
        m_playing = playing;
        apply_camera_mode();
    }

    auto bridge_game::reset_timer() -> void
    {
        m_timer = noi_engine::timer{};
    }

    auto bridge_game::set_key_pressed(const int key) -> void
    {
        m_held_keys.insert(key);
        this->input().set_key_pressed(static_cast<noi_engine::key_code>(key));
    }

    auto bridge_game::set_key_released(const int key) -> void
    {
        m_held_keys.erase(key);
        this->input().set_key_released(static_cast<noi_engine::key_code>(key));
    }

    auto bridge_game::release_all_keys() -> void
    {
        for (const auto key : m_held_keys)
        {
            this->input().set_key_released(static_cast<noi_engine::key_code>(key));
        }
        m_held_keys.clear();
    }

    auto bridge_game::on_resize(const int width, const int height) -> void
    {
        if (!this->get_current_scene())
        {
            return;
        }

        auto& cam = this->get_camera_2d_component();
        cam.orthographic_size = (static_cast<float>(height) / PIXELS_PER_UNIT) * 0.5f;
        cam.aspect_ratio = static_cast<float>(width) / static_cast<float>(height);

        this->m_scene_manager->on_resize(width, height);
    }

    auto bridge_game::load_scripts(const std::string& scripts_library_path) -> bool
    {
        if (m_scripts_library_handle)
        {
            return true;
        }

        if (!std::filesystem::exists(scripts_library_path))
        {
            std::fprintf(stderr,
                         "[noi_engine_editor_bridge] Game scripts library not found (build the project first): %s\n",
                         scripts_library_path.c_str());
            return false;
        }

        m_scripts_library_handle = dlopen(scripts_library_path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (!m_scripts_library_handle)
        {
            std::fprintf(stderr, "[noi_engine_editor_bridge] Failed to load game scripts library %s: %s\n",
                         scripts_library_path.c_str(), dlerror());
            return false;
        }

        const auto register_scripts = reinterpret_cast<register_scripts_fn>(
            dlsym(m_scripts_library_handle, REGISTER_SCRIPTS_SYMBOL));

        if (!register_scripts)
        {
            std::fprintf(stderr, "[noi_engine_editor_bridge] Game scripts library %s has no %s entry point\n",
                         scripts_library_path.c_str(), REGISTER_SCRIPTS_SYMBOL);
            dlclose(m_scripts_library_handle);
            m_scripts_library_handle = nullptr;
            return false;
        }

        register_scripts(*this);
        return true;
    }

    auto bridge_game::get_world() -> noi_engine::world&
    {
        const auto current_scene = this->get_current_scene();
        assert(current_scene);
        return current_scene->get_world();
    }

    auto bridge_game::get_current_scene() const -> noi_engine::scene*
    {
        return this->m_scene_manager->current();
    }

    auto bridge_game::get_camera_2d_component() -> noi_engine::camera_2d&
    {
        const auto camera_entity = this->get_current_scene()->get_camera_entity();
        auto& world = this->get_world();
        assert(world.has<noi_engine::camera_2d>(camera_entity));

        return world.get<noi_engine::camera_2d>(camera_entity);
    }
}
