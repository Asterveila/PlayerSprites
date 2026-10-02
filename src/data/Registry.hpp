#pragma once

#include <Geode/Result.hpp>

namespace playersprites {

	namespace fs = std::filesystem;

	class Registry {
	protected:
		fs::path m_registryPath;
		std::vector<std::string> m_packIds;

	public:
		static fs::path packsDir();
		static Registry loadOrCreate();

		geode::Result<void, std::string> save() const;

		std::vector<std::string> const& packIds() const { return m_packIds; }
		bool contains(std::string const& id) const;

		// Both are in-memory only until save() is called.
		bool addPack(std::string const& id);
		bool removePack(std::string const& id);
	};

}
