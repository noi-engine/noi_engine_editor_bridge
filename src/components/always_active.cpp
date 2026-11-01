#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/culling.hpp>

// "always_active": the entity is drawn and simulated even outside the camera's culling area.
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        auto save(const component_context&, bool, nlohmann::json& components) -> void
        {
            components.push_back({{"type", "always_active"}});
        }
    }

    auto make_always_active_handler() -> component_handler
    {
        return {
            .type = "always_active",
            .has = component_ops::has<noi_engine::always_active>,
            .add = component_ops::add<noi_engine::always_active>,
            .remove = component_ops::remove<noi_engine::always_active>,
            .copy = component_ops::copy<noi_engine::always_active>,
            .save = save,
        };
    }
}
