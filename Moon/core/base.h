#pragma once
#include <memory>


template<typename T>
inline constexpr bool const_false = false;

template<typename T> using asset_ref = std::shared_ptr<T>;
template<typename T> using asset_tmp = std::unique_ptr<T>;

template<typename T, typename... Args>
constexpr asset_ref<T> MakeAsset(Args&&... args) {
	return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
constexpr asset_tmp<T> WrapAsset(Args&&... args) {
	return std::make_unique<T>(std::forward<Args>(args)...);
}