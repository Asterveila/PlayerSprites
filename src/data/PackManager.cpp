#include "PackManager.hpp"
#include "Registry.hpp"
#include "../core/SoundPlayer.hpp"
#include <matjson.hpp>
#include <Geode/Geode.hpp>
#include <Geode/utils/file.hpp>

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

		if (!fs::exists(root) || !fs::is_directory(root)) {
			return Err(fmt::format("pack folder does not exist: {}", utils::string::pathToString(root)));
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

namespace playersprites::transfer {

	namespace {

		bool isPackFile(fs::path const& file) {
			auto ext = utils::string::pathToString(file.extension());
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			return ext == ".json" || ext == ".png" || ext == ".ogg" || ext == ".wav";
		}

		bool isThisBullshit(fs::path const& path) {
			for (auto const& part : path) {
				if (utils::string::pathToString(part) == "__MACOSX") return true;
			}
			return utils::string::pathToString(path.filename()).starts_with("._");
		}

		bool isSafeRelative(fs::path const& path) {
			if (path.empty() || path.is_absolute() || path.has_root_name() || path.has_root_directory()) return false;
			for (auto const& part : path) {
				if (part == fs::path("..")) return false;
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

		void removeFolder(fs::path const& folder) {
			std::error_code ec;
			fs::remove_all(folder, ec);
		}

	}

	Result<std::string, std::string> importPack(fs::path const& zipPath) {
		auto unzipRes = file::Unzip::create(zipPath);
		if (unzipRes.isErr()) {
			return Err(fmt::format("couldn't open zip: {}", unzipRes.unwrapErr()));
		}
		auto unzip = std::move(unzipRes).unwrap();

		auto entries = unzip.getEntries();
		if (entries.size() > 2000) {
			return Err(std::string("zip has wayyyy too many files in it to be a sprite pack. be careful what you're downloading."));
		}

		for (auto const& entry : entries) {
			if (!isSafeRelative(entry)) {
				return Err(fmt::format("zip contains unsafe path ({}), so it wasn't imported.", utils::string::pathToString(entry)));
			}
		}

		std::vector<fs::path> manifests;
		for (auto const& entry : entries) {
			if (utils::string::pathToString(entry.filename()) == "spr-pack.json" && !isThisBullshit(entry)) manifests.push_back(entry);
		}
		if (manifests.empty()) {
			return Err(std::string("zip has no spr-pack.json in it, so it isn't a valid sprite pack. contact pack creator if you think this is a mistake!"));
		}
		if (manifests.size() > 1) {
			return Err(std::string("zip has more than one pack in it. please import them one at a time."));
		}

		auto packRootInZip = manifests[0].parent_path();
		if (std::distance(packRootInZip.begin(), packRootInZip.end()) > 1) {
			return Err(std::string("spr-pack.json has to be at the top of the zip, or directly inside a single folder."));
		}

		auto& registry = PackManager::get().registry();
		auto baseName = packRootInZip.empty() ? utils::string::pathToString(zipPath.stem()) : utils::string::pathToString(packRootInZip);
		auto id = uniqueId(cleanupId(baseName), registry);
		auto destRoot = Registry::packsDir() / id;

		if (auto res = file::createDirectoryAll(destRoot); res.isErr()) {
			return Err(fmt::format("couldn't create pack's folder: {}", res.unwrapErr()));
		}

		for (auto const& entry : entries) {
			if (entry.filename().empty()) continue;

			auto rel = packRootInZip.empty() ? entry : entry.lexically_relative(packRootInZip);
			if (rel.empty() || *rel.begin() == fs::path("..")) continue;
			if (isThisBullshit(entry) || !isPackFile(rel)) continue;

			auto dest = (destRoot / rel).lexically_normal();
			auto fromRoot = dest.lexically_relative(destRoot);
			if (fromRoot.empty() || *fromRoot.begin() == fs::path("..")) continue;

			if (auto res = file::createDirectoryAll(dest.parent_path()); res.isErr()) {
				removeFolder(destRoot);
				return Err(fmt::format("Couldn't create a folder for {}: {}", utils::string::pathToString(rel), res.unwrapErr()));
			}
			if (auto res = unzip.extractTo(entry, dest); res.isErr()) {
				removeFolder(destRoot);
				return Err(fmt::format("Couldn't extract {}: {}", utils::string::pathToString(rel), res.unwrapErr()));
			}
		}

		registry.addPack(id);
		if (auto res = registry.save(); res.isErr()) {
			registry.removePack(id);
			removeFolder(destRoot);
			return Err(fmt::format("couldn't update registry.json: {}", res.unwrapErr()));
		}

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

	Result<fs::path, std::string> exportPack(std::string const& packId) {
		auto* pack = PackManager::get().findPack(packId);
		if (!pack) {
			return Err(std::string("that pack isn't loaded."));
		}

		auto exportDir = Mod::get()->getSettingValue<std::filesystem::path>("export-folder");
		if (auto res = file::createDirectoryAll(exportDir); res.isErr()) {
			return Err(fmt::format("couldn't create export folder: {}", res.unwrapErr()));
		}

		auto outPath = exportDir / fmt::format("{}.zip", packId);

		std::error_code ec;
		fs::remove(outPath, ec);

		std::optional<std::string> failure;
		size_t added = 0;

		{
			auto zipRes = file::Zip::create(outPath);
			if (zipRes.isErr()) {
				return Err(fmt::format("couldn't create zip: {}", zipRes.unwrapErr()));
			}
			auto zip = std::move(zipRes).unwrap();

			for (auto const& item : fs::recursive_directory_iterator(pack->rootPath, ec)) {
				if (!item.is_regular_file(ec)) continue;

				auto rel = item.path().lexically_relative(pack->rootPath);
				if (rel.empty() || isThisBullshit(rel) || !isPackFile(rel)) continue;

				if (auto res = zip.addFrom(item.path(), rel.parent_path()); res.isErr()) {
					failure = fmt::format("couldn't add {} to zip: {}", utils::string::pathToString(rel), res.unwrapErr());
					break;
				}
				added++;
			}
		}

		if (!failure && added == 0) failure = "this pack has no files to export.";
		if (failure) {
			fs::remove(outPath, ec);
			return Err(*failure);
		}

		return Ok(outPath);
	}

}

