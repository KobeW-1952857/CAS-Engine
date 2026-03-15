#pragma once

#include <tuple>

#include "renderer/material.h"
#include "renderer/primitives/mesh.h"
#include "renderer/shader.h"
#include "scene/scene.h"

class AssetManager;

template <typename T>
concept IsAsset = requires {
  { AssetTraits<T>::type } -> std::convertible_to<AssetType>;
  { AssetTraits<T>::default_name } -> std::convertible_to<const char*>;
  { AssetTraits<T>::extension } -> std::convertible_to<const char*>;
} && requires(std::string_view ext) {
  { AssetTraits<T>::matchesExtension(ext) } -> std::same_as<bool>;
} && requires(const std::filesystem::path& path, AssetManager& assets) {
  { AssetTraits<T>::load(path, assets) } -> std::same_as<std::shared_ptr<T>>;
} && requires(const T& asset, const std::filesystem::path& path, AssetManager& assets) {
  { AssetTraits<T>::save(asset, path, assets) } -> std::same_as<void>;
} && requires(T& asset, AssetManager& assets) {
  { AssetTraits<T>::initializeNew(asset, assets) } -> std::same_as<void>;
};

using AllAssetTypes = std::tuple<Mesh, Shader, Material, Scene>;

namespace detail {
template <typename T>
constexpr bool validateAsset() {
  static_assert(requires { AssetTraits<T>::type; });
  static_assert(requires { AssetTraits<T>::default_name; });
  static_assert(requires { AssetTraits<T>::extension; });
  static_assert(requires(std::string_view ext) { AssetTraits<T>::matchesExtension(ext); });
  static_assert(
      requires(const std::filesystem::path& path, AssetManager& assets) { AssetTraits<T>::load(path, assets); });
  static_assert(requires(const T& asset, const std::filesystem::path& path, AssetManager& assets) {
    AssetTraits<T>::save(asset, path, assets);
  });
  static_assert(requires(T& asset, AssetManager& assets) { AssetTraits<T>::initializeNew(asset, assets); });

  return true;
}
template <typename Tuple>
struct AllSatisfyAsset;

template <typename... Ts>
struct AllSatisfyAsset<std::tuple<Ts...>> {
  static constexpr bool value = (IsAsset<Ts> && ...);
};

template <typename F>
void forEachAssetType(F&& f) {
  std::apply([&](auto... dummy) { (f.template operator()<decltype(dummy)>(), ...); }, AllAssetTypes{});
}
}  // namespace detail

static_assert(detail::validateAsset<Mesh>());
static_assert(detail::validateAsset<Shader>());
static_assert(detail::validateAsset<Material>());
static_assert(detail::validateAsset<Scene>());

static_assert(detail::AllSatisfyAsset<AllAssetTypes>::value,
              "All types in AllAssetTypes must satisfy the IsAsset concept");
