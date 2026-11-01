#include "component_handler.hpp"

#include <nlohmann/json.hpp>
#include <noi_engine/core/components/camera_2d.hpp>

// For the scene's camera, everything goes through editor_data::authored_camera: the live
// camera_2d is stretched to the editor viewport, only zoom / near / far are kept in sync.
namespace noi_engine_editor_bridge_detail
{
    namespace
    {
        using noi_engine::camera_2d;

        [[nodiscard]] auto is_scene_camera(const component_context& c) -> bool
        {
            return c.entity == c.editor.camera_entity;
        }

        [[nodiscard]] auto shown_camera(const component_context& c) -> const camera_2d&
        {
            return is_scene_camera(c) ? c.editor.authored_camera : c.world.get<camera_2d>(c.entity);
        }

        auto enumerate(const component_context& c, const property_emitter& emitter) -> void
        {
            const auto& cam = shown_camera(c);
            emitter.emit_float("zoom", cam.zoom, false);
            emitter.emit_float("near_plane", cam.near_plane, false);
            emitter.emit_float("far_plane", cam.far_plane, false);
            emitter.emit_float("aspect_ratio", cam.aspect_ratio, false);
            emitter.emit_float("orthographic_size", cam.orthographic_size, false);
            // Lets the editor find the camera whose red border it can resize in the viewport.
            emitter.emit_bool("scene_camera", is_scene_camera(c), true);
        }

        auto set_value(camera_2d& cam, const std::string& name, const float v) -> bool
        {
            if (name == "zoom") { cam.zoom = v; }
            else if (name == "near_plane") { cam.near_plane = v; }
            else if (name == "far_plane") { cam.far_plane = v; }
            else if (name == "aspect_ratio") { cam.aspect_ratio = v; }
            else if (name == "orthographic_size") { cam.orthographic_size = v; }
            else { return false; }
            return true;
        }

        auto set_property(const component_context& c, const std::string& name,
                          const noi_engine_editor_bridge_property_value& value) -> bool
        {
            if (value.type != NOI_ENGINE_EDITOR_BRIDGE_PROPERTY_FLOAT)
            {
                return false;
            }
            const float v = value.number[0];

            if (!is_scene_camera(c))
            {
                return set_value(c.world.get<camera_2d>(c.entity), name, v);
            }

            if (!set_value(c.editor.authored_camera, name, v))
            {
                return false;
            }
            if (name == "zoom" || name == "near_plane" || name == "far_plane")
            {
                set_value(c.world.get<camera_2d>(c.entity), name, v);
            }
            return true;
        }

        auto save(const component_context& c, bool, nlohmann::json& components) -> void
        {
            const auto& cam = shown_camera(c);
            nlohmann::json props;
            props["zoom"] = cam.zoom;
            props["near_plane"] = cam.near_plane;
            props["far_plane"] = cam.far_plane;
            props["orthographic_size"] = cam.orthographic_size;
            // The parser only ever reads a width/height pair, not the resolved ratio -
            // reconstruct a pair that divides back to the same value.
            props["aspect_ratio_width"] = cam.aspect_ratio;
            props["aspect_ratio_height"] = 1.0f;
            components.push_back({{"type", "camera_2d"}, {"properties", props}});
        }
    }

    auto make_camera_2d_handler() -> component_handler
    {
        return {
            .type = "camera_2d",
            .has = component_ops::has<camera_2d>,
            .enumerate = enumerate,
            .set_property = set_property,
            .add = component_ops::add<camera_2d>,
            .remove = component_ops::remove<camera_2d>,
            .copy = component_ops::copy<camera_2d>,
            .save = save,
        };
    }
}
