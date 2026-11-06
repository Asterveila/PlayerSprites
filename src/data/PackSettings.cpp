#include "PackSettings.hpp"

namespace playersprites::settings {

	// -----------------------------------------
	// HELPERS START
	// -----------------------------------------

	std::string packKey(std::string const& packId) {
		return fmt::format("pack.{}.enabled", packId);
	}
	std::string modeKey(std::string const& packId, bool platformer) {
		return fmt::format("mode.{}.{}.enabled", packId, platformer ? "platformer" : "classic");
	}
	std::string gamemodeKey(std::string const& packId, std::string const& gamemode) {
		return fmt::format("gamemode.{}.{}.enabled", packId, gamemode);
	}
	std::string eventKey(std::string const& packId, std::string const& gamemode, std::string const& eventId) {
		return fmt::format("event.{}.{}.{}.enabled", packId, gamemode, eventId);
	}

	std::string tauntKey(std::string const& packId, std::string const& gamemode, std::string const& tauntId) {
		return fmt::format("taunt.{}.{}.{}.enabled", packId, gamemode, tauntId);
	}

	std::string sfxKey(std::string const& packId) {
		return fmt::format("sfx.{}.enabled", packId);
	}
	std::string soundKey(std::string const& packId, std::string const& gamemode, std::string const& eventName) {
		return fmt::format("sound.{}.{}.{}.enabled", packId, gamemode, eventName);
	}

	// -----------------------------------------
	// HELPERS END
	// -----------------------------------------

	bool isPackEnabled(std::string const& packId) {
		return geode::Mod::get()->getSavedValue<bool>(packKey(packId), true);
	}
	void setPackEnabled(std::string const& packId, bool enabled) {
		isPackEnabled(packId);
		geode::Mod::get()->setSavedValue<bool>(packKey(packId), enabled);
	}

	bool isModeEnabled(std::string const& packId, bool platformer) {
		return geode::Mod::get()->getSavedValue<bool>(modeKey(packId, platformer), true);
	}
	void setModeEnabled(std::string const& packId, bool platformer, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(modeKey(packId, platformer), enabled);
	}

	bool isPackActive(std::string const& packId, bool platformer) {
		return isPackEnabled(packId) && isModeEnabled(packId, platformer);
	}

	bool isGamemodeEnabled(std::string const& packId, std::string const& gamemode) {
		return geode::Mod::get()->getSavedValue<bool>(gamemodeKey(packId, gamemode), true);
	}
	void setGamemodeEnabled(std::string const& packId, std::string const& gamemode, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(gamemodeKey(packId, gamemode), enabled);
	}

	bool isEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId) {
		return geode::Mod::get()->getSavedValue<bool>(eventKey(packId, gamemode, eventId), true);
	}
	void setEventEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventId, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(eventKey(packId, gamemode, eventId), enabled);
	}

	bool isSfxEnabled(std::string const& packId) {
		return geode::Mod::get()->getSavedValue<bool>(sfxKey(packId), true);
	}
	void setSfxEnabled(std::string const& packId, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(sfxKey(packId), enabled);
	}

	bool isSoundEnabled(std::string const& packId, std::string const& gamemode, std::string const& entryId, bool defaultValue) {
		return geode::Mod::get()->getSavedValue<bool>(soundKey(packId, gamemode, entryId), defaultValue);
	}
	void setSoundEnabled(std::string const& packId, std::string const& gamemode, std::string const& eventName, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(soundKey(packId, gamemode, eventName), enabled);
	}

	bool isTauntEnabled(std::string const& packId, std::string const& gamemode, std::string const& tauntId) {
		return geode::Mod::get()->getSavedValue<bool>(tauntKey(packId, gamemode, tauntId), true);
	}
	void setTauntEnabled(std::string const& packId, std::string const& gamemode, std::string const& tauntId, bool enabled) {
		geode::Mod::get()->setSavedValue<bool>(tauntKey(packId, gamemode, tauntId), enabled);
	}

	void clearPack(std::string const& packId) {
		auto& save = geode::Mod::get()->getSaveContainer();

		std::vector<std::string> exact = { packKey(packId), sfxKey(packId) };
		std::vector<std::string> prefixes = {
			fmt::format("mode.{}.", packId),
			fmt::format("gamemode.{}.", packId),
			fmt::format("event.{}.", packId),
			fmt::format("taunt.{}.", packId),
			fmt::format("sound.{}.", packId)
		};

		std::vector<std::string> doomed;
		for (auto const& [key, value] : save) {
			std::string k = std::string(key);
			bool match = std::find(exact.begin(), exact.end(), k) != exact.end();
			for (auto const& prefix : prefixes) {
				if (k.starts_with(prefix)) { match = true; break; }
			}
			if (match) doomed.push_back(k);
		}
		for (auto const& k : doomed) save.erase(k);
	}

}
