#pragma once

#include <algorithm>
#include <cstdio>
#include <string>

#include <glm/glm.hpp>
#include <nlohmann/json.hpp>

namespace noi_engine_editor_bridge_detail
{
    // Inverse of noi_engine::color_parser::parse() ("#RRGGBBAA").
    inline auto to_hex_string(const glm::vec4& c) -> std::string
    {
        const auto byte = [](const float v) -> unsigned
        {
            return static_cast<unsigned>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        };

        char buf[10];
        std::snprintf(buf, sizeof(buf), "#%02x%02x%02x%02x", byte(c.r), byte(c.g), byte(c.b), byte(c.a));
        return buf;
    }

    inline auto vec3_json(const glm::vec3& v) -> nlohmann::json
    {
        return nlohmann::json::array({v.x, v.y, v.z});
    }
}
