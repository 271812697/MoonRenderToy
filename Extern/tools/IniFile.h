#pragma once

#include <string>
#include <unordered_map>


namespace Tools::Filesystem
{

	class IniFile final
	{
	public:
		using AttributePair = std::pair<std::string, std::string>;
		using AttributeMap = std::unordered_map<std::string, std::string>;


		IniFile(const std::string& p_filePath);

		void Reload();

		void Rewrite() const;


		template<typename T>
		T Get(const std::string& p_key);


		template<typename T>
		T GetOrDefault(const std::string& p_key, T p_default);

		template<typename T>
		bool Set(const std::string& p_key, const T& p_value);

		template<typename T>
		bool Add(const std::string& p_key, const T& p_value);

		bool Remove(const std::string& p_key);

		void RemoveAll();

		bool IsKeyExisting(const std::string& p_key) const;


		std::vector<std::string> GetFormattedContent() const;

	private:
		void RegisterPair(const std::string& p_key, const std::string& p_value);
		void RegisterPair(const AttributePair& p_pair);

		void Load();

		AttributePair	ExtractKeyAndValue(const std::string& p_attributeLine)	const;
		bool			IsValidLine(const std::string& p_attributeLine)	const;
		bool			StringToBoolean(const std::string& p_value)			const;

	private:
		std::string		m_filePath;
		AttributeMap	m_data;
	};
    	template<typename T>
	inline T IniFile::Get(const std::string& p_key)
	{
		if constexpr (std::is_same<bool, T>::value)
		{
			if (!IsKeyExisting(p_key))
				return false;

			return StringToBoolean(m_data[p_key]);
		}
		else if constexpr (std::is_same<std::string, T>::value)
		{
			if (!IsKeyExisting(p_key))
				return std::string("NULL");

			return m_data[p_key];
		}
		else if constexpr (std::is_integral<T>::value)
		{
			if (!IsKeyExisting(p_key))
				return static_cast<T>(0);

			return static_cast<T>(std::atoi(m_data[p_key].c_str()));
		}
		else if constexpr (std::is_floating_point<T>::value)
		{
			if (!IsKeyExisting(p_key))
				return static_cast<T>(0.0f);

			return static_cast<T>(std::atof(m_data[p_key].c_str()));
		}
		else
		{
			static_assert(false, "The given type must be : bool, integral, floating point or string");
			return T();
		}
	}

	template<typename T>
	inline T IniFile::GetOrDefault(const std::string& p_key, T p_default)
	{
		return IsKeyExisting(p_key) ? Get<T>(p_key) : p_default;
	}

	template<typename T>
	inline bool IniFile::Set(const std::string& p_key, const T& p_value)
	{
		if (IsKeyExisting(p_key))
		{
			if constexpr (std::is_same<bool, T>::value)
			{
				m_data[p_key] = p_value ? "true" : "false";
			}
			else if constexpr (std::is_same<std::string, T>::value)
			{
				m_data[p_key] = p_value;
			}
			else if constexpr (std::is_integral<T>::value)
			{
				m_data[p_key] = std::to_string(p_value);
			}
			else if constexpr (std::is_floating_point<T>::value)
			{
				m_data[p_key] = std::to_string(p_value);
			}
			else
			{
				static_assert(false, "The given type must be : bool, integral, floating point or string");
			}

			return true;
		}

		return false;
	}

	template<typename T>
	inline bool IniFile::Add(const std::string & p_key, const T & p_value)
	{
		if (!IsKeyExisting(p_key))
		{
			if constexpr (std::is_same<bool, T>::value)
			{
				RegisterPair(p_key, p_value ? "true" : "false");
			}
			else if constexpr (std::is_same<std::string, T>::value)
			{
				RegisterPair(p_key, p_value);
			}
			else if constexpr (std::is_integral<T>::value)
			{
				RegisterPair(p_key, std::to_string(p_value));
			}
			else if constexpr (std::is_floating_point<T>::value)
			{
				RegisterPair(p_key, std::to_string(p_value));
			}
			else
			{
				static_assert(false, "The given type must be : bool, integral, floating point or std::string");
			}

			return true;
		}

		return false;
	}
}