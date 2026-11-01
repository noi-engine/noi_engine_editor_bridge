#pragma once

#include "noi_engine_editor_bridge/bridge_api.h"

#include <glm/glm.hpp>
#include <string>

namespace noi_engine_editor_bridge_detail
{
    // Builds noi_engine_editor_bridge_property_value entries and forwards them to an editor
    // callback. Shared by the entity/component and material property enumerators.
    struct property_emitter
    {
        noi_engine_editor_bridge_property_entry_callback callback;
        void* user_data;

        auto emit_float(const char* name, const float v, const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT;
            pv.number[0] = v;
            send(name, pv, read_only);
        }

        auto emit_vec3(const char* name, const glm::vec3& v, const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_VEC3;
            pv.number[0] = v.x;
            pv.number[1] = v.y;
            pv.number[2] = v.z;
            send(name, pv, read_only);
        }

        auto emit_vec4(const char* name, const glm::vec4& v, const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_VEC4;
            pv.number[0] = v.x;
            pv.number[1] = v.y;
            pv.number[2] = v.z;
            pv.number[3] = v.w;
            send(name, pv, read_only);
        }

        auto emit_string(const char* name, const std::string& s, const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_STRING;
            pv.text = s.c_str();
            send(name, pv, read_only);
        }

        auto emit_bool(const char* name, const bool b, const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_BOOL;
            pv.flag = b ? 1 : 0;
            send(name, pv, read_only);
        }

        auto emit_resource_ref(const char* name, const std::string& label,
                               const noi_engine_editor_bridge_resource_type resource_type,
                               const bool read_only) const -> void
        {
            noi_engine_editor_bridge_property_value pv{};
            pv.type = NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_RESOURCE_REF;
            pv.text = label.c_str();
            pv.resource_type = resource_type;
            send(name, pv, read_only);
        }

    private:
        auto send(const char* name, const noi_engine_editor_bridge_property_value& pv, const bool read_only) const
            -> void
        {
            callback(user_data, name, pv, read_only ? 1 : 0);
        }
    };
}
