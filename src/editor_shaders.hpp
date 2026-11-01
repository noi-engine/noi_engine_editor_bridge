#pragma once

// Shaders of the editor-only overlay (see bridge_game_overlay.cpp). Embedded here so a game
// project never ships them in its own resources.
namespace noi_engine_editor_bridge_detail::editor_shaders
{
    inline constexpr auto QUAD_VERTEX = R"(#version 330 core

layout(location = 0) in vec3 a_position;
layout(location = 2) in vec2 a_uv;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_uv;

void main()
{
    v_uv = a_uv;
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0);
}
)";

    // Red border showing the game camera's bounds.
    inline constexpr auto CAMERA_GIZMO_FRAGMENT = R"(#version 330 core

in vec2 v_uv;

out vec4 FragColor;

uniform vec4 u_color;
uniform float u_border_thickness_x;
uniform float u_border_thickness_y;

void main()
{
    vec2 dist_to_edge = min(v_uv, 1.0 - v_uv);

    if (dist_to_edge.x > u_border_thickness_x && dist_to_edge.y > u_border_thickness_y)
    {
        discard;
    }

    FragColor = u_color;
}
)";
}
