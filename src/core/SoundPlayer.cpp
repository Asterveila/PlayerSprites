#include "SoundPlayer.hpp"
#include "../data/PackManager.hpp"
#include "../data/PackSettings.hpp"

#include <Geode/Geode.hpp>
#include <chrono>
#include <random>
#include <unordered_map>
#include <unordered_set>

namespace playersprites::audio {

	namespace {

		std::unordered_set<std::string>& preloadedPaths() {
			static std::unordered_set<std::string> paths;
			return paths;
		}

		struct SequenceState {
			size_t next = 0;
			bool hasPlayed = false;
			std::chrono::steady_clock::time_point lastPlayed;
		};

		std::unordered_map<std::string, SequenceState>& sequences() {
			static std::unordered_map<std::string, SequenceState> states;
			return states;
		}

		std::string enginePath(fs::path const& file) {
			return geode::utils::string::pathToString(file);
		}

		fs::path const& pickFile(std::string const& packId, std::string const& gamemode, SoundEntry const& entry) {
			size_t count = entry.files.size();

			if (!entry.ordered || count == 1) {
				static std::mt19937 gen{ std::random_device{}() };
				return entry.files[std::uniform_int_distribution<size_t>(0, count - 1)(gen)];
			}

			auto& state = sequences()[fmt::format("{}/{}/{}", packId, gamemode, entry.id)];
			auto now = std::chrono::steady_clock::now();

			if (entry.resetAfter > 0.f && state.hasPlayed) {
				float secondsSince = std::chrono::duration<float>(now - state.lastPlayed).count();
				if (secondsSince > entry.resetAfter) state.next = 0;
			}
			if (state.next >= count) state.next = 0;

			auto const& file = entry.files[state.next];
			state.next = (state.next + 1) % count;
			state.hasPlayed = true;
			state.lastPlayed = now;
			return file;
		}

	}

	void playForEvent(bool platformer, std::string const& gamemode, std::string const& eventName) {
		for (auto const& pack : PackManager::get().getLoadedPacks()) {
			auto entries = pack.sounds.matching(gamemode, eventName);
			if (entries.empty()) continue;

			if (!settings::isPackActive(pack.id, platformer)) continue;
			if (!settings::isSfxEnabled(pack.id)) continue;

			bool playedAny = false;
			for (auto const* entry : entries) {
				if (entry->files.empty()) continue;
				if (!settings::isSoundEnabled(pack.id, gamemode, entry->id, entry->defaultEnabled)) continue;

				playFile(pickFile(pack.id, gamemode, *entry));
				playedAny = true;
			}

			if (playedAny) return;
		}
	}

	void playFile(fs::path const& file) {
		FMODAudioEngine::sharedEngine()->playEffect(enginePath(file));
	}

	void resetSequences() {
		sequences().clear();
	}

	void preloadAll() {
		auto* engine = FMODAudioEngine::sharedEngine();

		for (auto const& pack : PackManager::get().getLoadedPacks()) {
			if (pack.sounds.empty()) continue;
			if (!settings::isPackEnabled(pack.id) || !settings::isSfxEnabled(pack.id)) continue;

			for (auto const& [gamemode, entries] : pack.sounds.gamemodes) {
				for (auto const& [entryId, entry] : entries) {
					for (auto const& file : entry.files) {
						auto path = enginePath(file);
						if (preloadedPaths().insert(path).second) {
							engine->preloadEffect(path);
						}
					}
				}
			}
		}
	}

	void unloadAll() {
		sequences().clear();

		if (preloadedPaths().empty()) return;

		auto* engine = FMODAudioEngine::sharedEngine();
		for (auto const& path : preloadedPaths()) {
			engine->unloadEffect(path);
		}
		preloadedPaths().clear();
	}

}
