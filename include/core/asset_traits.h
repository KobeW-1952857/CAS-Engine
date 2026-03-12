#pragma once

class AssetManager;

// ─── AssetTraits<T> ──────────────────────────────────────────────────────────
//
// Each concrete asset type must provide a full specialization of this struct.
// The base template is intentionally left undefined: a missing specialization
// will produce a clear "incomplete type" error at compile time rather than
// silently falling through.
//
// Required members per specialization:
//
//   static constexpr AssetType type
//     The AssetType enum value that identifies this type at runtime.
//
//   static constexpr const char* default_name
//     Stem used when creating a new unnamed asset (e.g. "New Material").
//
//   static constexpr const char* extension
//     File extension including the dot (e.g. ".casmat").
//
//   static bool matchesExtension(std::string_view ext)
//     Returns true if the given file extension maps to this asset type.
//     One type may claim multiple extensions (e.g. Shader claims .vert/.frag).
//
//   static std::shared_ptr<T> load(const std::filesystem::path& path,
//                                  AssetManager& assets)
//     Loads and constructs an asset from an absolute path.
//     AssetManager& is provided so types like Material and Shader can resolve
//     their own sub-asset handles (shaders, textures, etc.).
//
//   static void initializeNew(T& asset, AssetManager& assets)
//     Called by AssetManager::createNewAsset<T>() after default-constructing
//     the asset but before serializing it to disk. Use this to set up sensible
//     defaults (e.g. assign a default shader to a new Material).
//     Leave the body empty for types that need no initialization.

template <typename T>
struct AssetTraits;