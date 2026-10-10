#pragma once

#include "Registry.hpp"
#include "SpritePackTypes.hpp"
#include <Geode/Result.hpp>

namespace playersprites {

	class PackManager {
	protected:
		Registry m_registry;
		std::vector<SpritePack> m_loadedPacks;
		std::vector<std::pair<std::string, std::string>> m_loadErrors; // id -> message

		geode::Result<SpritePack, std::string> loadOne(std::string const& id);

	public:
		static PackManager& get();

		void reload();

		Registry& registry() { return m_registry; }

		std::vector<SpritePack> const& getLoadedPacks() const { return m_loadedPacks; }
		std::vector<std::pair<std::string, std::string>> const& getLoadErrors() const { return m_loadErrors; }

		SpritePack const* findPack(std::string const& id) const;
	};

}

namespace playersprites::transfer {

	geode::Result<std::string, std::string> importPack(std::filesystem::path const& zipPath);
	geode::Result<std::filesystem::path, std::string> exportPack(std::string const& packId);

}