#pragma once

#include <Geode/Result.hpp>
#include <asp/fs.hpp>

namespace playersprites {

	namespace fs = asp::fs;

	class Registry {
	protected:
		fs::Path m_registryPath;
		std::vector<std::string> m_packIds;

	public:
		static fs::Path packsDir();
		static Registry loadOrCreate();

		geode::Result<void, std::string> save() const;

		std::vector<std::string> const& packIds() const { return m_packIds; }
		bool contains(std::string const& id) const;

		bool addPack(std::string const& id, bool top = false);
		bool removePack(std::string const& id);
		bool swapPacks(std::string const& a, std::string const& b);
	};

}
