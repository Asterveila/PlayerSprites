#include "PackManager.hpp"
#include "../core/SoundPlayer.hpp"
#include <matjson.hpp>

namespace playersprites {

	// using geode::Ok;
	using geode::Err;
	using geode::Result;

	PackManager& PackManager::get() {
		static PackManager instance;
		return instance;
	}

	Result<SpritePack, std::string> PackManager::loadOne(std::string const& id) {
		auto root = Registry::packsDir() / id;

		if (!fs::exists(root) || !fs::is_directory(root)) {
			return Err(fmt::format("pack folder does not exist: {}", root.string()));
		}

		auto packJsonPath = root / "spr-pack.json";
		// @geode-ignore(unknown-resource)
		auto packPngPath = root / "pack.png";

		if (!fs::exists(packPngPath)) {
			// @geode-ignore(unknown-resource)
			return Err(std::string("missing pack.png"));
		}
		if (!fs::exists(packJsonPath)) {
			return Err(std::string("missing spr-pack.json"));
		}

		auto readRes = geode::utils::file::readString(packJsonPath);
		if (readRes.isErr()) {
			return Err(fmt::format("could not read spr-pack.json: {}", readRes.unwrapErr()));
		}

		auto parseRes = matjson::parse(readRes.unwrap());
		if (parseRes.isErr()) {
			return Err(fmt::format("spr-pack.json is not valid json: {}", parseRes.unwrapErr()));
		}

		auto packRes = SpritePack::parse(id, root, parseRes.unwrap());
		if (packRes.isErr()) return Err(packRes.unwrapErr());
		auto pack = packRes.unwrap();

		// sounds.json is optional
		auto soundsPath = root / "sounds.json";
		if (fs::exists(soundsPath)) {
			auto soundsRead = geode::utils::file::readString(soundsPath);
			if (soundsRead.isErr()) {
				return Err(fmt::format("could not read sounds.json: {}", soundsRead.unwrapErr()));
			}
			auto soundsParse = matjson::parse(soundsRead.unwrap());
			if (soundsParse.isErr()) {
				return Err(fmt::format("sounds.json is not valid json: {}", soundsParse.unwrapErr()));
			}
			auto soundsRes = SoundSet::parse(root, soundsParse.unwrap());
			if (soundsRes.isErr()) return Err(soundsRes.unwrapErr());
			pack.sounds = soundsRes.unwrap();
		}

		return geode::Ok(std::move(pack));
	}

	void PackManager::reload() {
		audio::unloadAll();

		m_registry = Registry::loadOrCreate();
		m_loadedPacks.clear();
		m_loadErrors.clear();

		for (auto const& id : m_registry.packIds()) {
			auto res = loadOne(id);
			if (res.isOk()) {
				m_loadedPacks.push_back(res.unwrap());
			} else {
				m_loadErrors.emplace_back(id, res.unwrapErr());
				geode::log::warn("failed to load pack '{}': {}", id, res.unwrapErr());
			}
		}

		geode::log::info("loaded {} pack(s), {} failed", m_loadedPacks.size(), m_loadErrors.size());
	}

	SpritePack const* PackManager::findPack(std::string const& id) const {
		for (auto const& pack : m_loadedPacks) {
			if (pack.id == id) return &pack;
		}
		return nullptr;
	}

}
