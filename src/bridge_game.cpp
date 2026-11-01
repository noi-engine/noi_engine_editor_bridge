#include "bridge_game.hpp"

#include <cassert>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

#include <dlfcn.h>
#include <unistd.h>

#include <noi_engine/core/components/camera_2d.hpp>
#include <noi_engine/core/model/script.hpp>
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
        if (m_playing)
        {
            refresh_camera_gizmo(); // follows the camera the game moves
        }

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
        // Scripts only run in Play mode: while editing, the scene stays as authored.
        if (auto* current = this->get_current_scene())
        {
            current->set_scripts_paused(!playing);
        }
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

    // Loads the project's scripts library, or reloads it after the project was recompiled. The
    // library is copied to a unique temporary file first: dlopen() of a path already loaded
    // returns the old library, and the build may overwrite the file while it is mapped.
    // Scripts register themselves when the library loads (NOI_ENGINE_SCRIPT); the optional
    // noi_engine_register_scripts entry point adds the custom ones. A previously loaded library
    // is never unloaded: instances of its scripts may still be alive (the editor reloads the
    // scene right after, which replaces them).
    auto bridge_game::load_scripts(const std::string& scripts_library_path) -> bool
    {
        namespace fs = std::filesystem;

        std::error_code error;
        if (!fs::exists(scripts_library_path, error))
        {
            std::fprintf(stderr,
                         "[noi_engine_editor_bridge] Game scripts library not found (build the project first): %s\n",
                         scripts_library_path.c_str());
            return false;
        }

        const auto copy_path = fs::temp_directory_path(error) /
            ("noi_engine_editor_scripts_" + std::to_string(::getpid()) + "_" + std::to_string(++m_scripts_load_count) +
             fs::path(scripts_library_path).extension().string());
        if (error || !fs::copy_file(scripts_library_path, copy_path, fs::copy_options::overwrite_existing, error))
        {
            std::fprintf(stderr, "[noi_engine_editor_bridge] Couldn't copy the scripts library %s: %s\n",
                         scripts_library_path.c_str(), error.message().c_str());
            return false;
        }

        // Only the new library's scripts must stay registered (a deleted script disappears),
        // but keep the old ones if it fails to load.
        auto& registry = noi_engine::details::script_registry::instance();
        const auto previous_factories = registry.factories();
        registry.clear();

        void* handle = dlopen(copy_path.c_str(), RTLD_NOW | RTLD_LOCAL);
        fs::remove(copy_path, error); // the mapping stays valid
        if (!handle)
        {
            std::fprintf(stderr, "[noi_engine_editor_bridge] Failed to load game scripts library %s: %s\n",
                         scripts_library_path.c_str(), dlerror());
            for (const auto& [name, factory] : previous_factories)
            {
                registry.add(name, factory);
            }
            return false;
        }

        if (const auto register_scripts = reinterpret_cast<register_scripts_fn>(dlsym(handle, REGISTER_SCRIPTS_SYMBOL)))
        {
            register_scripts(*this);
        }

        m_scripts_library_handle = handle;
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
