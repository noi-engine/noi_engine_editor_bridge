cmake_minimum_required(VERSION 3.23)

# Versions live in bridge.json (read by the editor and the release workflow too).
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/bridge.json" BRIDGE_JSON)
string(JSON BRIDGE_VERSION GET "${BRIDGE_JSON}" version)
string(JSON BRIDGE_API_VERSION GET "${BRIDGE_JSON}" api_version)
string(JSON BRIDGE_MIN_ENGINE GET "${BRIDGE_JSON}" noi_engine min)
string(JSON BRIDGE_MAX_ENGINE GET "${BRIDGE_JSON}" noi_engine max)

project(noi_engine_editor_bridge VERSION ${BRIDGE_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# A prebuilt noi_engine (a release zip's noi_engine/ folder: include/ + libs/), ideally the
# bridge.json "min" version.
set(NOI_ENGINE_ROOT "" CACHE PATH "Path to a prebuilt noi_engine (folder with include/ and libs/)")
if (NOT EXISTS "${NOI_ENGINE_ROOT}/include/noi_engine/noi_engineConfig.hpp")
    message(FATAL_ERROR "Set NOI_ENGINE_ROOT to a prebuilt noi_engine ${BRIDGE_MIN_ENGINE} (folder with include/ and libs/)")
endif ()

find_package(glm CONFIG REQUIRED)

configure_file(.templates/bridge_config.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/noi_engine_editor_bridge_config.hpp @ONLY)

add_library(noi_engine_editor_bridge SHARED
        include/noi_engine_editor_bridge/bridge_api.h
        src/bridge_api.cpp
        src/bridge_game.hpp
        src/bridge_game.cpp
        src/bridge_game_overlay.cpp
        src/bridge_game_entities.cpp
        src/bridge_game_components.cpp
        src/bridge_game_resources.cpp
        src/bridge_game_persistence.cpp
        src/editor_shaders.hpp

        src/properties/property_emitter.hpp
        src/properties/property_value.hpp
        src/persistence/json_utils.hpp

        src/components/component_handler.hpp
        src/components/component_registry.hpp
        src/components/component_registry.cpp
        src/components/box_collider.cpp
        src/components/camera_2d.cpp
        src/components/camera_3d.cpp
        src/components/gravity.cpp
        src/components/mesh_renderer.cpp
        src/components/mesh_renderer_properties.hpp
        src/components/mesh_renderer_properties.cpp
        src/components/name_component.cpp
        src/components/orbit_camera.cpp
        src/components/render_layer.cpp
        src/components/script_component.cpp
        src/components/transform.cpp
        src/components/world_matrix.cpp
)

target_include_directories(noi_engine_editor_bridge PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party
        ${CMAKE_CURRENT_BINARY_DIR}/generated
        ${NOI_ENGINE_ROOT}/include
)

target_link_directories(noi_engine_editor_bridge PRIVATE ${NOI_ENGINE_ROOT}/libs)
target_link_libraries(noi_engine_editor_bridge PRIVATE noi_engine glm::glm)

# The bridge never ships noi_engine: the editor loads the project's own libnoi_engine first,
# so the bridge and the game's scripts share a single engine instance.
set_target_properties(noi_engine_editor_bridge PROPERTIES
        BUILD_RPATH "${NOI_ENGINE_ROOT}/libs"
        INSTALL_RPATH ""
)

install(TARGETS noi_engine_editor_bridge
        LIBRARY DESTINATION libs
        RUNTIME DESTINATION libs
)
install(FILES bridge.json DESTINATION .)
install(FILES include/noi_engine_editor_bridge/bridge_api.h DESTINATION include/noi_engine_editor_bridge)
