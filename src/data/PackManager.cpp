#include "PackManager.hpp"
#include "Registry.hpp"
#include "PackSettings.hpp"
#include "../core/SoundPlayer.hpp"
#include <matjson.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>
#include <asp/fs/std.hpp>

using namespace geode::prelude;

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

		if (!root.isDir()) {
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
			
			auto tauntSounds = std::move(pack.sounds);
			pack.sounds = soundsRes.unwrap();
			for (auto& [gamemode, entries] : tauntSounds.gamemodes) {
				for (auto& [entryId, entry] : entries) {
					pack.sounds.gamemodes[gamemode].emplace(entryId, std::move(entry));
				}
			}
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

	bool PackManager::movePack(std::string const& id, int direction) {
		auto it = std::find_if(m_loadedPacks.begin(), m_loadedPacks.end(), [&](SpritePack const& pack) { return pack.id == id; });
		if (it == m_loadedPacks.end()) return false;

		auto other = it;
		if (direction < 0) {
			if (it == m_loadedPacks.begin()) return false;
			other = it - 1;
		} else {
			if (it + 1 == m_loadedPacks.end()) return false;
			other = it + 1;
		}

		if (!m_registry.swapPacks(it->id, other->id)) return false;
		if (auto res = m_registry.save(); res.isErr()) {
			m_registry.swapPacks(it->id, other->id);
			geode::log::error("couldn't save registry.json after reordering: {}", res.unwrapErr());
			return false;
		}

		std::iter_swap(it, other);
		return true;
	}

	Result<void, std::string> PackManager::deletePack(std::string const& id) {
		if (!m_registry.contains(id)) return Err(std::string("that pack isn't in the registry."));

		auto packsDir = Registry::packsDir().lexicallyNormal();
		auto root = (packsDir / id).lexicallyNormal();
		auto rootParent = root.parent();
		if (id.empty() || !rootParent || *rootParent != packsDir.view()) {
			return Err(fmt::format("'{}' doesn't seem to point to a folder inside the packs folder. nothing was deleted just to be safe.", id));
		}

		audio::unloadAll();

		auto backup = m_registry;
		m_registry.removePack(id);
		if (auto res = m_registry.save(); res.isErr()) {
			m_registry = backup;
			return Err(fmt::format("couldn't update registry.json: {}", res.unwrapErr()));
		}

		settings::clearPack(id);

		auto removeRes = fs::removeDirAll(root, false);

		this->reload();

		if (removeRes.isErr()) {
			return Err(fmt::format(
				"the pack was removed from your registry, but its folder couldn't be fully deleted ({}). if u wanna delete it by yourself: {}",
				removeRes.unwrapErr(), root.string()
			));
		}
		return Ok();
	}

}

namespace playersprites::transfer {

	// -----------------------------------------
	// HELPERS START
	// -----------------------------------------

	bool isPackFile(fs::Path const& file) {
		auto ext = std::string(file.extension().value_or(""));
		std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
		return ext == "json" || ext == "png" || ext == "ogg" || ext == "wav";
	}

	bool isThisBullshit(fs::Path const& path) {
		for (auto part : path.components()) {
			if (part == "__MACOSX") return true;
		}
		return path.filename().value_or("").view().starts_with("._");
	}

	bool isSafeRelative(fs::Path const& path) {
		auto pathView = path.view().view();
		if (path.empty() || path.isAbsolute() || path.hasRootName() || pathView.starts_with('/') || pathView.starts_with('\\')) return false;
		for (auto part : path.components()) {
			if (part == "..") return false;
		}
		return true;
	}

	std::string cleanupId(std::string name) {
		for (auto& c : name) {
			bool ok = std::isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == ' ' || c == '.';
			if (!ok) c = '_';
		}
		auto first = name.find_first_not_of(". ");
		if (first == std::string::npos) return "";
		auto last = name.find_last_not_of(". ");
		return name.substr(first, last - first + 1);
	}

	std::string uniqueId(std::string base, Registry const& registry) {
		if (base.empty()) base = "imported-pack";

		auto id = base;
		for (int n = 2; fs::exists(Registry::packsDir() / id) || registry.contains(id); ++n) {
			id = fmt::format("{}-{}", base, n);
		}
		return id;
	}

	void removeFolder(fs::Path const& folder) {
		(void) fs::removeDirAll(folder, false);
	}

	// -----------------------------------------
	// HELPERS END
	// -----------------------------------------

	Result<std::string, std::string> importPack(fs::Path const& zipPath) {
		auto unzipRes = file::Unzip::create(zipPath);
		if (unzipRes.isErr()) {
			return Err(fmt::format("couldn't open zip: {}", unzipRes.unwrapErr()));
		}
		auto unzip = std::move(unzipRes).unwrap();

		auto entries = unzip.getEntries();
		if (entries.size() > 3000) {
			return Err(std::string("zip has wayyyy too many files in it to be a sprite pack. be careful what you're downloading."));
		}

		for (auto const& entry : entries) {
			if (!isSafeRelative(entry)) {
				return Err(fmt::format("zip contains unsafe path ({}), so it wasn't imported.", entry.string()));
			}
		}

		std::vector<fs::Path> manifests;
		for (auto const& entry : entries) {
			if (entry.filename().value_or("").view() == "spr-pack.json" && !isThisBullshit(entry)) manifests.push_back(entry);
		}
		if (manifests.empty()) {
			return Err(std::string("zip has no spr-pack.json in it, so it isn't a valid sprite pack. contact pack creator if you think this is a mistake!"));
		}
		if (manifests.size() > 1) {
			return Err(std::string("zip has more than one pack in it. please import them one at a time."));
		}

		fs::Path packRootInZip = manifests[0].parent().value_or("");
		if (packRootInZip.components().count() > 1) {
			return Err(std::string("spr-pack.json has to be at the top of the zip, or directly inside a single folder."));
		}

		auto& registry = PackManager::get().registry();
		auto baseName = packRootInZip.empty() ? std::string(zipPath.stem().value_or("")) : packRootInZip.string();
		auto id = uniqueId(cleanupId(baseName), registry);
		auto destRoot = Registry::packsDir() / id;

		if (auto res = file::createDirectoryAll(destRoot); res.isErr()) {
			return Err(fmt::format("couldn't create pack's folder: {}", res.unwrapErr()));
		}

		for (auto const& entry : entries) {
			if (!entry.filename()) continue;

			auto relOpt = packRootInZip.empty() ? std::optional<fs::Path>{ entry } : entry.lexicallyRelative(packRootInZip);
			if (!relOpt || relOpt->empty() || relOpt->startsWith("..")) continue;
			auto const& rel = *relOpt;
			if (isThisBullshit(entry) || !isPackFile(rel)) continue;

			auto dest = (destRoot / rel).lexicallyNormal();
			auto fromRoot = dest.lexicallyRelative(destRoot);
			if (!fromRoot || fromRoot->empty() || fromRoot->startsWith("..")) continue;

			if (auto res = file::createDirectoryAll(dest.parent().value_or("")); res.isErr()) {
				removeFolder(destRoot);
				return Err(fmt::format("couldn't create a folder for {}: {}", rel.string(), res.unwrapErr()));
			}
			if (auto res = unzip.extractTo(entry, dest); res.isErr()) {
				removeFolder(destRoot);
				return Err(fmt::format("couldn't extract {}: {}", rel.string(), res.unwrapErr()));
			}
		}

		registry.addPack(id, true);
		if (auto res = registry.save(); res.isErr()) {
			registry.removePack(id);
			removeFolder(destRoot);
			return Err(fmt::format("couldn't update registry.json: {}", res.unwrapErr()));
		}

		settings::setPackEnabled(id, false);

		PackManager::get().reload();

		for (auto const& [errorId, loadError] : PackManager::get().getLoadErrors()) {
			if (errorId != id) continue;

			std::string message = loadError;

			auto& freshRegistry = PackManager::get().registry();
			freshRegistry.removePack(id);
			(void)freshRegistry.save();
			removeFolder(destRoot);
			PackManager::get().reload();

			return Err(fmt::format("pack isn't valid: ({}) so it wasn't installed.", message));
		}

		return Ok(id);
	}

	Result<fs::Path, std::string> exportPack(std::string const& packId) {
		auto* pack = PackManager::get().findPack(packId);
		if (!pack) {
			return Err(std::string("that pack isn't loaded."));
		}

		// @geode-ignore(unknown-setting)
		auto exportDir = Mod::get()->getSettingValue<fs::Path>("export-folder");
		if (auto res = file::createDirectoryAll(exportDir); res.isErr()) {
			return Err(fmt::format("couldn't create export folder: {}", res.unwrapErr()));
		}

		auto outPath = exportDir / fmt::format("{}.zip", packId);

		(void) fs::remove(outPath);

		std::optional<std::string> failure;
		size_t added = 0;

		{
			auto zipRes = file::Zip::create(outPath);
			if (zipRes.isErr()) {
				return Err(fmt::format("couldn't create zip: {}", zipRes.unwrapErr()));
			}
			auto zip = std::move(zipRes).unwrap();

			auto iterRes = fs::iterDirRecursive(pack->rootPath);
			if (iterRes.isErr()) {
				failure = fmt::format("couldn't read the pack's folder: {}", iterRes.unwrapErr());
			} else {
				auto iterator = std::move(iterRes).unwrap();

				for (auto entryRes : iterator) {
					if (entryRes.isErr()) {
						failure = fmt::format("couldn't read the pack's folder: {}", entryRes.unwrapErr());
						break;
					}
					auto& item = entryRes.unwrap();
					if (!item.isFile()) continue;

					auto rel = item.path.lexicallyRelative(pack->rootPath);
					if (!rel || rel->empty() || isThisBullshit(*rel) || !isPackFile(*rel)) continue;

					if (auto res = zip.addFrom(item.path, rel->parent().value_or("")); res.isErr()) {
						failure = fmt::format("couldn't add {} to zip: {}", rel->string(), res.unwrapErr());
						break;
					}
					added++;
				}
			}
		}

		if (!failure && added == 0) failure = "this pack has no files to export.";
		if (failure) {
			(void) fs::remove(outPath);
			return Err(*failure);
		}

		return Ok(outPath);
	}

	Result<size_t, std::string> installBuiltinPacks() {
		static std::vector<std::string> const builtinPacks = {
			"cute-lil-bow.zip",
			"zero-x3.zip",
			"zero-x4.zip",
			"kris-ch5.zip",
			"sonic-3.zip",
			"sonic-1.zip",
			"sonicmania.zip"
		};

		auto const resourcesDir = Mod::get()->getResourcesDir();

		size_t installed = 0;
		std::string failures;
		for (auto const& name : builtinPacks) {
			auto zipPath = resourcesDir / name;

			if (!fs::exists(zipPath)) {
				failures += fmt::format("\n- {}: file is missing", name);
				continue;
			}

			auto res = importPack(zipPath);
			if (res.isOk()) {
				installed++;
			} else {
				failures += fmt::format("\n- {}: {}", name, res.unwrapErr());
			}
		}

		if (installed == 0) return Err(fmt::format("none of the built-in packs could be installed:{}", failures));
		if (!failures.empty()) log::warn("some built-in packs failed to install:{}", failures);
		return Ok(installed);
	}

}

