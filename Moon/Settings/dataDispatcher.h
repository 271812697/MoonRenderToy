#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
namespace MOON {
template <typename T>
class DataDispatcher {
public:
	void RegisterReference(T& reference) {
		mDataPointer = &reference;
	}
	void RegisterProvider(std::function<void(T)>provider) {
		mProvider = provider;
	}
	void RegisterGather(std::function<T(void)>gather) {
		mGatherer = gather;
	}
	void Provide(const T& data) {
		if (mValueChange) {
			if (mDataPointer) {
				*mDataPointer = data;
			}
			else
			{
				mProvider(data);
			}
			mValueChange = false;
		}
	}
	void NotifyChange() {
		mValueChange = true;
	}
	T Gather() {
		return mDataPointer ? *mDataPointer : mGatherer();
	}
private:
	bool mValueChange = false;
	T* mDataPointer = nullptr;
	std::function<void(T)>mProvider;
	std::function<T(void)>mGatherer;
};
}