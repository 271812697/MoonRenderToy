#include<utility>
#include "asset.h"


namespace asset {

	IAsset::IAsset() : id(0) {

	}

	IAsset::IAsset(IAsset&& other) noexcept : id{ std::exchange(other.id, 0) } {}

	IAsset& IAsset::operator=(IAsset&& other) noexcept {
		if (this != &other) {
			this->id = std::exchange(other.id, 0);
		}
		return *this;
	}

}
