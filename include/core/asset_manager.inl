#pragma once

#include <Nexus/Log.h>

#include <string>
#include <string_view>

#include "core/asset_manager.h"
#include "core/asset_traits.h"
#include "core/asset_type_list.h"
#include "utils/filesystem.h"

namespace detail {
template <typename Tuple, typename F, std::size_t... Is>
void for_each_asset_type(F&& f, std::index_sequence<Is...>) {
  (f.template operator()<std::tuple_element_t<Is, Tuple>>(), ...);
}

template <typename Tuple, typename F>
void for_each_asset_type(F&& f) {
  for_each_asset_type<Tuple>(std::forward<F>(f), std::make_index_sequence<std::tuple_size_v<Tuple>>{});
}
}  // namespace detail

inline AssetType AssetManager::getTypeFromExtension(std::string_view ext) const {
  AssetType result = AssetType::None;
  detail::for_each_asset_type<AllAssetTypes>([&]<typename T> {
    if (result == AssetType::None && AssetTraits<T>::matchesExtension(ext)) result = AssetTraits<T>::type;
  });
  return result;
}

template <typename T>
std::vector<UUID> AssetManager::getAssetsOfType() {
  std::vector<UUID> assets;
  for (auto [id, meta] : s_asset_registry)
    if (meta.type == AssetTraits<T>::type) assets.push_back(id);
  return assets;
}

template <typename T>
std::unordered_map<UUID, AssetMetadata> AssetManager::getAssetsMetadataOfType() {
  std::unordered_map<UUID, AssetMetadata> assets;
  for (auto [id, meta] : s_asset_registry)
    if (meta.type == AssetTraits<T>::type) assets.emplace(id, meta);
  return assets;
}

template <typename T>
std::shared_ptr<T> AssetManager::getAsset(UUID handle) {
  if (!s_asset_registry.contains(handle)) return nullptr;
  if (s_loaded_assets.contains(handle)) return std::dynamic_pointer_cast<T>(s_loaded_assets[handle]);
  return loadAsset<T>(handle);
}

template <typename T>
std::shared_ptr<T> AssetManager::getDefaultAsset() {
  constexpr AssetType t = AssetTraits<T>::type;
  if (!s_default_assets.contains(t)) return nullptr;
  return getAsset<T>(s_default_assets[t]);
}

template <typename T>
UUID AssetManager::createNewAsset(const std::filesystem::path& virtual_directory) {
  std::filesystem::path virtual_path =
      virtual_directory / (std::string(AssetTraits<T>::default_name) + AssetTraits<T>::extension);
  std::filesystem::path absolute_path = m_filesystem.resolvePath(virtual_path);

  int counter = 1;
  while (std::filesystem::exists(absolute_path)) {
    virtual_path = virtual_directory / (std::string(AssetTraits<T>::default_name) + "_" + std::to_string(counter) +
                                        AssetTraits<T>::extension);
    absolute_path = m_filesystem.resolvePath(virtual_path);
    counter++;
  }

  auto new_asset = std::make_shared<T>();
  AssetTraits<T>::initializeNew(*new_asset, *this);

  // new_asset->serialize(absolute_path);
  AssetTraits<T>::save(*new_asset, absolute_path, *this);

  UUID new_id;
  new_asset->handle = new_id;

  AssetMetadata meta{
      .handle = new_id,
      .type = getAssetType<T>(),
      .filepath = virtual_path,
      .is_loaded = true,
  };

  Nexus::Logger::debug("Creating new asset at {}", meta.filepath.string());
  s_asset_registry[new_id] = meta;
  s_loaded_assets[new_id] = new_asset;

  serialize();
  return new_id;
}

template <typename T>
std::shared_ptr<T> AssetManager::loadAsset(UUID handle) {
  const auto& metadata = s_asset_registry[handle];
  auto path = m_filesystem.resolvePath(metadata.filepath);

  std::shared_ptr<T> asset = AssetTraits<T>::load(path, *this);
  if (!asset) return nullptr;

  asset->handle = handle;
  s_loaded_assets[handle] = asset;
  s_asset_registry[handle].is_loaded = true;
  return std::dynamic_pointer_cast<T>(asset);
}

template <typename T>
AssetType AssetManager::getAssetType() {
  return AssetTraits<T>::type;
}

// --- Asset Type traits -----------------------------------------------------------------------------------------------
inline std::shared_ptr<Shader> AssetTraits<Shader>::load(const std::filesystem::path& path, AssetManager& assets) {
  return std::make_shared<Shader>(path, assets.filesystem());
}

inline void AssetTraits<Material>::initializeNew(Material& asset, AssetManager& assets) {
  asset.shader = assets.getDefaultAsset<Shader>();
}