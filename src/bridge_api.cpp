// extern "C" glue exposing noi_engine_editor_bridge_detail::bridge_game (bridge_game.hpp and its
// bridge_game*.cpp implementations) through the stable ABI declared in bridge_api.h. Kept
// deliberately thin: every function here just forwards to a bridge_game method.

#include "bridge_game.hpp"

#include <noi_engine/noi_engineConfig.hpp>
#include <noi_engine_editor_bridge_config.hpp>

static_assert(NOI_ENGINE_EDITOR_BRIDGE_MANIFEST_API_VERSION == NOI_ENGINE_EDITOR_BRIDGE_API_VERSION,
              "bridge.json api_version must match NOI_ENGINE_EDITOR_BRIDGE_API_VERSION in bridge_api.h");

using noi_engine_editor_bridge_detail::bridge_game;

void noi_engine_editor_bridge_init_gl(const noi_engine_editor_bridge_gl_proc_loader loader)
{
    noi_engine::game::init(loader);
}

noi_engine_editor_bridge_game_handle noi_engine_editor_bridge_create(
    const int game_width, const int game_height, const char* resources_path_utf8)
{
    auto* game = new bridge_game(game_width, game_height, std::string(resources_path_utf8));
    return reinterpret_cast<noi_engine_editor_bridge_game_handle>(game);
}

void noi_engine_editor_bridge_destroy(const noi_engine_editor_bridge_game_handle handle)
{
    delete reinterpret_cast<bridge_game*>(handle);
}

void noi_engine_editor_bridge_run(const noi_engine_editor_bridge_game_handle handle)
{
    reinterpret_cast<bridge_game*>(handle)->run();
}

void noi_engine_editor_bridge_on_resize(const noi_engine_editor_bridge_game_handle handle, const int width, const int height)
{
    reinterpret_cast<bridge_game*>(handle)->on_resize(width, height);
}

void noi_engine_editor_bridge_set_scene_2d(const noi_engine_editor_bridge_game_handle handle, const char* scene_path_utf8)
{
    reinterpret_cast<bridge_game*>(handle)->set_scene_2d(scene_path_utf8);
}

int noi_engine_editor_bridge_load_scripts(const noi_engine_editor_bridge_game_handle handle,
                                           const char* scripts_library_path_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->load_scripts(scripts_library_path_utf8) ? 1 : 0;
}

const char* noi_engine_editor_bridge_engine_version(void)
{
    return NOI_ENGINE_VERSION;
}

int noi_engine_editor_bridge_api_version(void)
{
    return NOI_ENGINE_EDITOR_BRIDGE_API_VERSION;
}

const char* noi_engine_editor_bridge_version(void)
{
    return NOI_ENGINE_EDITOR_BRIDGE_VERSION;
}

void noi_engine_editor_bridge_enumerate_resources(const noi_engine_editor_bridge_game_handle handle,
                                                   const noi_engine_editor_bridge_resource_entry_callback callback,
                                                   void* const user_data)
{
    reinterpret_cast<bridge_game*>(handle)->enumerate_resources(callback, user_data);
}

void noi_engine_editor_bridge_enumerate_entities(const noi_engine_editor_bridge_game_handle handle,
                                                  const noi_engine_editor_bridge_entity_entry_callback callback,
                                                  void* const user_data)
{
    reinterpret_cast<bridge_game*>(handle)->enumerate_entities(callback, user_data);
}

void noi_engine_editor_bridge_enumerate_component_properties(
    const noi_engine_editor_bridge_game_handle handle, const uint32_t entity_id, const uint32_t entity_generation,
    const char* component_type_utf8, const noi_engine_editor_bridge_property_entry_callback callback,
    void* const user_data)
{
    reinterpret_cast<bridge_game*>(handle)->enumerate_component_properties(
        entity_id, entity_generation, component_type_utf8 ? component_type_utf8 : "", callback, user_data);
}

int noi_engine_editor_bridge_set_component_property(
    const noi_engine_editor_bridge_game_handle handle, const uint32_t entity_id, const uint32_t entity_generation,
    const char* component_type_utf8, const char* property_name_utf8,
    const noi_engine_editor_bridge_property_value value)
{
    return reinterpret_cast<bridge_game*>(handle)->set_component_property(
               entity_id, entity_generation, component_type_utf8 ? component_type_utf8 : "",
               property_name_utf8 ? property_name_utf8 : "", value)
               ? 1
               : 0;
}

int noi_engine_editor_bridge_add_component(const noi_engine_editor_bridge_game_handle handle,
                                            const uint32_t entity_id, const uint32_t entity_generation,
                                            const char* component_type_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->add_component(
               entity_id, entity_generation, component_type_utf8 ? component_type_utf8 : "")
               ? 1
               : 0;
}

void noi_engine_editor_bridge_enumerate_material_properties(
    const noi_engine_editor_bridge_game_handle handle, const uint32_t material_id, const uint32_t material_generation,
    const noi_engine_editor_bridge_property_entry_callback callback, void* const user_data)
{
    reinterpret_cast<bridge_game*>(handle)->enumerate_material_properties(material_id, material_generation, callback,
                                                                           user_data);
}

int noi_engine_editor_bridge_set_material_property(const noi_engine_editor_bridge_game_handle handle,
                                                    const uint32_t material_id, const uint32_t material_generation,
                                                    const char* property_name_utf8,
                                                    const noi_engine_editor_bridge_property_value value)
{
    return reinterpret_cast<bridge_game*>(handle)->set_material_property(
               material_id, material_generation, property_name_utf8 ? property_name_utf8 : "", value)
               ? 1
               : 0;
}

void noi_engine_editor_bridge_set_key_pressed(const noi_engine_editor_bridge_game_handle handle, const int key_code)
{
    reinterpret_cast<bridge_game*>(handle)->set_key_pressed(key_code);
}

void noi_engine_editor_bridge_set_key_released(const noi_engine_editor_bridge_game_handle handle, const int key_code)
{
    reinterpret_cast<bridge_game*>(handle)->set_key_released(key_code);
}

void noi_engine_editor_bridge_release_all_keys(const noi_engine_editor_bridge_game_handle handle)
{
    reinterpret_cast<bridge_game*>(handle)->release_all_keys();
}

void noi_engine_editor_bridge_set_playing(const noi_engine_editor_bridge_game_handle handle, const int playing)
{
    reinterpret_cast<bridge_game*>(handle)->set_playing(playing != 0);
}

void noi_engine_editor_bridge_reset_timer(const noi_engine_editor_bridge_game_handle handle)
{
    reinterpret_cast<bridge_game*>(handle)->reset_timer();
}

int noi_engine_editor_bridge_save_scene(const noi_engine_editor_bridge_game_handle handle, const char* target_path_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->save_scene(target_path_utf8 ? target_path_utf8 : "") ? 1 : 0;
}

void noi_engine_editor_bridge_set_grid_visible(const noi_engine_editor_bridge_game_handle handle, const int visible)
{
    reinterpret_cast<bridge_game*>(handle)->set_grid_visible(visible != 0);
}

noi_engine_editor_bridge_entity_ref noi_engine_editor_bridge_create_entity(
    const noi_engine_editor_bridge_game_handle handle, const char* name_utf8)
{
    const auto e = reinterpret_cast<bridge_game*>(handle)->create_entity(name_utf8 ? name_utf8 : "");
    return {e.m_id, e.m_generation};
}

int noi_engine_editor_bridge_destroy_entity(const noi_engine_editor_bridge_game_handle handle,
                                             const uint32_t entity_id, const uint32_t entity_generation)
{
    return reinterpret_cast<bridge_game*>(handle)->destroy_entity(entity_id, entity_generation) ? 1 : 0;
}

noi_engine_editor_bridge_entity_ref noi_engine_editor_bridge_duplicate_entity(
    const noi_engine_editor_bridge_game_handle handle, const uint32_t entity_id, const uint32_t entity_generation,
    const char* new_name_utf8)
{
    const auto e = reinterpret_cast<bridge_game*>(handle)->duplicate_entity(
        entity_id, entity_generation, new_name_utf8 ? new_name_utf8 : "");
    return {e.m_id, e.m_generation};
}

int noi_engine_editor_bridge_remove_component(const noi_engine_editor_bridge_game_handle handle,
                                               const uint32_t entity_id, const uint32_t entity_generation,
                                               const char* component_type_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->remove_component(
               entity_id, entity_generation, component_type_utf8 ? component_type_utf8 : "")
               ? 1
               : 0;
}

int noi_engine_editor_bridge_remove_component_property(const noi_engine_editor_bridge_game_handle handle,
                                                         const uint32_t entity_id, const uint32_t entity_generation,
                                                         const char* component_type_utf8,
                                                         const char* property_name_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->remove_component_property(
               entity_id, entity_generation, component_type_utf8 ? component_type_utf8 : "",
               property_name_utf8 ? property_name_utf8 : "")
               ? 1
               : 0;
}

int noi_engine_editor_bridge_remove_material_property(const noi_engine_editor_bridge_game_handle handle,
                                                        const uint32_t material_id, const uint32_t material_generation,
                                                        const char* property_name_utf8)
{
    return reinterpret_cast<bridge_game*>(handle)->remove_material_property(
               material_id, material_generation, property_name_utf8 ? property_name_utf8 : "")
               ? 1
               : 0;
}
