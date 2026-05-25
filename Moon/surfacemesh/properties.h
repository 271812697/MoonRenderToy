#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <utility>
#include <vector>
#include "surfacemesh/exceptions.h"

namespace MOON {

	class BasePropertyArray
	{
	public:
		virtual ~BasePropertyArray() = default;

		virtual void reserve(size_t n) = 0;

		virtual void resize(size_t n) = 0;

		virtual void free_memory() = 0;

		virtual void push_back() = 0;

		virtual void swap(size_t i0, size_t i1) = 0;

		virtual BasePropertyArray* clone() const = 0;

		virtual const std::string& name() const = 0;
	};

	template <class T>
	class PropertyArray : public BasePropertyArray
	{
	public:
		using ValueType = T;
		using VectorType = std::vector<ValueType>;
		using reference = typename VectorType::reference;
		using const_reference = typename VectorType::const_reference;

		PropertyArray(std::string name, T t = T())
			: name_(std::move(name)), value_(std::move(t))
		{
		}

		void reserve(size_t n) override { data_.reserve(n); }

		void resize(size_t n) override { data_.resize(n, value_); }

		void push_back() override { data_.push_back(value_); }

		void free_memory() override { data_.shrink_to_fit(); }

		void swap(size_t i0, size_t i1) override
		{
			T d(data_[i0]);
			data_[i0] = data_[i1];
			data_[i1] = d;
		}

		BasePropertyArray* clone() const override
		{
			auto* p = new PropertyArray<T>(name_, value_);
			p->data_ = data_;
			return p;
		}

		const T* data() const { return &data_[0]; }

		std::vector<T>& vector() { return data_; }

		reference operator[](size_t idx)
		{
			assert(idx < data_.size());
			return data_[idx];
		}

		const_reference operator[](size_t idx) const
		{
			assert(idx < data_.size());
			return data_[idx];
		}

		const std::string& name() const override { return name_; }

	private:
		std::string name_;
		VectorType data_;
		ValueType value_;
	};

	template <>
	inline const bool* PropertyArray<bool>::data() const
	{
		assert(false);
		return nullptr;
	}

	template <class T>
	class Property
	{
	public:
		using reference = typename PropertyArray<T>::reference;
		using const_reference = typename PropertyArray<T>::const_reference;

		friend class PropertyContainer;
		friend class SurfaceMesh;

		explicit Property(PropertyArray<T>* p = nullptr) : parray_(p) {}

		void reset() { parray_ = nullptr; }

		explicit operator bool() const { return parray_ != nullptr; }

		reference operator[](size_t i)
		{
			assert(parray_ != nullptr);
			return (*parray_)[i];
		}

		const_reference operator[](size_t i) const
		{
			assert(parray_ != nullptr);
			return (*parray_)[i];
		}

		const T* data() const
		{
			assert(parray_ != nullptr);
			return parray_->data();
		}

		std::vector<T>& vector()
		{
			assert(parray_ != nullptr);
			return parray_->vector();
		}

	private:
		PropertyArray<T>& array()
		{
			assert(parray_ != nullptr);
			return *parray_;
		}

		const PropertyArray<T>& array() const
		{
			assert(parray_ != nullptr);
			return *parray_;
		}

		PropertyArray<T>* parray_;
	};

	class PropertyContainer
	{
	public:
		PropertyContainer() = default;

		virtual ~PropertyContainer() { clear(); }

		PropertyContainer(const PropertyContainer& rhs) { operator=(rhs); }

		PropertyContainer& operator=(const PropertyContainer& rhs)
		{
			if (this != &rhs)
			{
				clear();
				parrays_.resize(rhs.n_properties());
				size_ = rhs.size();
				for (size_t i = 0; i < parrays_.size(); ++i)
					parrays_[i] = rhs.parrays_[i]->clone();
			}
			return *this;
		}

		size_t size() const { return size_; }

		size_t n_properties() const { return parrays_.size(); }

		std::vector<std::string> properties() const
		{
			std::vector<std::string> names;
			names.reserve(parrays_.size());
			for (const auto* array : parrays_)
				names.emplace_back(array->name());
			return names;
		}

		template <class T>
		Property<T> add(const std::string& name, const T t = T())
		{
			for (const auto* parray : parrays_)
			{
				if (parray->name() == name)
				{
					const auto msg = "[PropertyContainer] A property with name \"" +
						name + "\" already exists.\n";
					throw InvalidInputException(msg);
				}
			}

			auto* p = new PropertyArray<T>(name, t);
			p->resize(size_);
			parrays_.push_back(p);
			return Property<T>(p);
		}

		bool exists(const std::string& name) const
		{
			for (auto parray : parrays_)
				if (parray->name() == name)
					return true;
			return false;
		}

		template <class T>
		Property<T> get(const std::string& name) const
		{
			for (auto parray : parrays_)
				if (parray->name() == name)
					return Property<T>(dynamic_cast<PropertyArray<T>*>(parray));
			return Property<T>();
		}

		template <class T>
		Property<T> get_or_add(const std::string& name, const T t = T())
		{
			Property<T> p = get<T>(name);
			if (!p)
				p = add<T>(name, t);
			return p;
		}

		template <class T>
		void remove(Property<T>& h)
		{
			auto it = parrays_.begin(), end = parrays_.end();
			for (; it != end; ++it)
			{
				if (*it == h.parray_)
				{
					delete* it;
					parrays_.erase(it);
					h.reset();
					break;
				}
			}
		}

		void clear()
		{
			for (auto& parray : parrays_)
				delete parray;
			parrays_.clear();
			size_ = 0;
		}

		void reserve(size_t n) const
		{
			for (auto parray : parrays_)
				parray->reserve(n);
		}

		void resize(size_t n)
		{
			for (auto& parray : parrays_)
				parray->resize(n);
			size_ = n;
		}

		void free_memory() const
		{
			for (auto parray : parrays_)
				parray->free_memory();
		}

		void push_back()
		{
			for (auto& parray : parrays_)
				parray->push_back();
			++size_;
		}

		void swap(size_t i0, size_t i1) const
		{
			for (auto parray : parrays_)
				parray->swap(i0, i1);
		}
	private:
		std::vector<BasePropertyArray*> parrays_;
		size_t size_{ 0 };
	};
}
