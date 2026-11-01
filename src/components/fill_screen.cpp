#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/fill_screen.hpp>
#include <noi_engine/core/components/transform.hpp>

// "fill_screen": the entity always covers what the camera sees, the whole window (see
// noi_engine/core/components/fill_screen.hpp). Made for backgrounds.
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        auto save(const component_context&, bool, nlohmann::json& components) -> void
        {
            components.push_back({{"type", "fill_screen"}});
        }

        // Needs a transform; never on the scene camera.
        auto add(const component_context& c) -> bool
        {
            return c.entity != c.editor.camera_entity && c.world.has<noi_engine::transform>(c.entity) &&
                component_ops::add<noi_engine::fill_screen>(c);
        }
    }

    auto make_fill_screen_handler() -> component_handler
    {
        return {
            .type = "fill_screen",
            .has = component_ops::has<noi_engine::fill_screen>,
            .add = add,
            .remove = component_ops::remove<noi_engine::fill_screen>,
            .copy = component_ops::copy<noi_engine::fill_screen>,
            .save = save,
        };
    }
}
