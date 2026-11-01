#include "component_handler.hpp"

#include <noi_engine/core/components/world_matrix.hpp>

namespace noi_engine_editor_bridge_detail
{
    // Engine-managed: recomputed from transform every frame. Listed in the Hierarchy but it
    // exposes no properties and can't be added, removed or copied by hand.
    auto make_world_matrix_handler() -> component_handler
    {
        return {
            .type = "world_matrix",
            .has = component_ops::has<noi_engine::world_matrix>,
        };
    }
}
