#pragma once

#include <string>

#include <noi_engine/core/resources/resources.hpp>

namespace noi_engine_editor_bridge_detail
{
    // Looks a resource up by label. resources::get_handle() returns an empty handle - which is
    // the *first* loaded resource of that type - for an unknown label, so check the label
    // round-trips instead of trusting it. Leaves `out` untouched and returns false when the
    // resource doesn't exist.
    template <typename T>
    [[nodiscard]] auto find_resource(const noi_engine::resources& resources, const std::string& label,
                                     noi_engine::resource_handle<T>& out) -> bool
    {
        if (label.empty())
        {
            return false;
        }
        const auto handle = resources.get_handle<T>(label);
        if (resources.get_label<T>(handle) != label)
        {
            return false;
        }
        out = handle;
        return true;
    }
}
