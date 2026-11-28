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

		bool movePack(std::string const& id, int direction);
		geode::Result<void, std::string> deletePack(std::string const& id);
	};

}

namespace playersprites::transfer {

	inline constexpr int BUILTIN_CURRENTVER = 4;

	geode::Result<std::string, std::string> importPack(fs::Path const& zipPath, std::optional<std::string> const& overwriteId = std::nullopt);
	geode::Result<fs::Path, std::string> exportPack(std::string const& packId);
	
	geode::Result<size_t, std::string> installBuiltinPacks();
	geode::Result<size_t, std::string> updateBuiltinPacks();

}