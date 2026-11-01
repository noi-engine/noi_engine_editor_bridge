# Noi Engine — Editor Bridge

The shared library that lets the [Noi Engine editor](https://github.com/noi-engine/noi_engine_editor) run a game's
scene live: viewport rendering, the Hierarchy, the Inspector and scene saving all go through it.

It is **editor-only**. Game projects never build, link or ship it.

## How it fits together

```
noi_engine_editor ──(C API, bridge_api.h)──▶ libnoi_engine_editor_bridge ──▶ libnoi_engine (the project's own copy)
                                                     │
                                                     └──▶ libgame_scripts (the project's scripts)
```

- The editor only talks to the bridge through the C functions in
  [`bridge_api.h`](include/noi_engine_editor_bridge/bridge_api.h). It never links noi_engine itself.
- The bridge doesn't ship noi_engine. The editor first loads the project's `.noi_engine/libs/libnoi_engine`, then the
  bridge, so the bridge and the game's scripts share one engine instance.
- Templates say which bridge they need (`editor_bridge_version` in `noi_engine.template.json`), and the editor downloads
  it from this repo's releases, next to the engine and the resources compiler.

## Versions: `bridge.json`

```json
{
  "version": "0.0.1",
  "api_version": 1,
  "noi_engine": { "min": "0.0.109", "max": "0.0.109" }
}
```

- `version`: this bridge's release version. The release tag must be `v<version>`.
- `noi_engine.min` / `noi_engine.max`: the engine versions this bridge works with. It is built against `min`, so it
  only supports versions that don't break the engine's ABI. The editor refuses to open a project whose
  `engine_version` is outside this range.
- `api_version`: must equal `NOI_ENGINE_EDITOR_BRIDGE_API_VERSION` in `bridge_api.h` (checked at compile time). Bump
  both on any breaking change to the C API. The editor refuses bridges with another API version.

## Releasing

Every noi_engine release needs a bridge release:

- **Non-breaking engine release** (new features, fixes): raise `noi_engine.max`, bump `version`.
- **Breaking engine release**: set both `min` and `max` to the new engine version, bump `version`, and fix the code if
  needed.

Then push the tag: `git tag v0.0.2 && git push origin v0.0.2`. The [`release`](.github/workflows/release.yml) workflow
downloads noi_engine `min` from its releases, builds the bridge for Linux x64 and macOS arm64 and attaches
`noi_engine_editor_bridge-<os>-<arch>.zip` to the release. Finally, update `editor_bridge_version` in the templates.

## Code layout

```
src/
├── bridge_api.cpp                 # the extern "C" functions, forwarding to bridge_game
├── bridge_game.hpp / .cpp         # game loop, input, resize, loading the scripts library
├── bridge_game_overlay.cpp        # scene loading + editor-only camera border and grid
├── bridge_game_entities.cpp       # entity listing, create / destroy / duplicate
├── bridge_game_components.cpp     # forwards component calls to the right handler
├── bridge_game_resources.cpp      # resources and materials
├── bridge_game_persistence.cpp    # saves the scene back to JSON
├── editor_shaders.hpp             # shaders of the editor overlay
├── components/                    # one file per component type
├── properties/                    # property values exchanged with the editor
└── persistence/                   # JSON helpers
```

To support a new component in the editor, add `src/components/<type>.cpp` and register it in
`src/components/component_registry.cpp`.

## Building locally

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset debug -DNOI_ENGINE_ROOT=/path/to/noi_engine   # a noi_engine release zip's noi_engine/ folder
cmake --build --preset debug
```

## Dependencies

[glm](https://github.com/g-truc/glm) and [nlohmann/json](https://github.com/nlohmann/json), both from vcpkg.
