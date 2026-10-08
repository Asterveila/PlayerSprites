#include "SpritePackTypes.hpp"
#include "EventShortcuts.hpp"
#include <random>

namespace playersprites {

	using geode::Ok;
	using geode::Err;
	using geode::Result;

	namespace {

		Result<std::string, std::string> requireString(matjson::Value const& json, std::string const& field, std::string const& ctx) {
			auto const& val = json[field];
			if (val.isNull()) {
				return Err(fmt::format("{}: missing required field '{}'", ctx, field));
			}
			auto res = val.asString();
			if (res.isErr()) {
				return Err(fmt::format("{}: field '{}' must be a string", ctx, field));
			}
			return Ok(res.unwrap());
		}

		Result<int, std::string> requireInt(matjson::Value const& json, std::string const& field, std::string const& ctx) {
			auto const& val = json[field];
			if (val.isNull()) {
				return Err(fmt::format("{}: missing required field '{}'", ctx, field));
			}
			auto res = val.asInt();
			if (res.isErr()) {
				return Err(fmt::format("{}: field '{}' must be an integer", ctx, field));
			}
			return Ok(static_cast<int>(res.unwrap()));
		}

		Result<std::optional<float>, std::string> optionalFloat(matjson::Value const& json, std::string const& field, std::string const& ctx) {
			auto const& val = json[field];
			if (val.isNull()) {
				return Ok(std::optional<float>(std::nullopt));
			}
			auto res = val.asDouble();
			if (res.isErr()) {
				return Err(fmt::format("{}: field '{}' must be a number", ctx, field));
			}
			return Ok(std::optional<float>(static_cast<float>(res.unwrap())));
		}

		Result<std::vector<std::string>, std::string> stringList(matjson::Value const& value, std::string const& what) {
			std::vector<std::string> out;
			if (value.isString()) {
				out.push_back(value.asString().unwrap());
			} else if (value.isArray()) {
				for (auto const& item : value) {
					auto strRes = item.asString();
					if (strRes.isErr()) {
						return Err(fmt::format("{}: every entry must be a string", what));
					}
					out.push_back(strRes.unwrap());
				}
			} else {
				return Err(fmt::format("{}: expected a string or an array of strings", what));
			}
			return Ok(std::move(out));
		}

		Result<std::optional<fs::path>, std::string> resolveSoundFile(fs::path const& normalRoot, std::string const& rel, std::string const& ctx) {
			fs::path relPath = rel;

			auto ext = relPath.extension().string();
			std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
			if (ext != ".ogg" && ext != ".wav") {
				return Err(fmt::format("{}: '{}' isn't a supported format, only .ogg and .wav work", ctx, rel));
			}

			if (relPath.is_absolute() || relPath.has_root_name() || relPath.has_root_directory()) {
				return Err(fmt::format("{}: '{}' has to be a path relative to the pack's folder", ctx, rel));
			}

			auto full = (normalRoot / relPath).lexically_normal();
			auto fromRoot = full.lexically_relative(normalRoot);
			if (fromRoot.empty() || *fromRoot.begin() == fs::path("..")) {
				return Err(fmt::format("{}: '{}' points outside of the pack's folder", ctx, rel));
			}

			if (!fs::exists(full)) {
				geode::log::warn("{}: missing file {}", ctx, full.string());
				return Ok(std::optional<fs::path>{});
			}
			return Ok(std::optional<fs::path>{ std::move(full) });
		}

		bool optionalBool(matjson::Value const& json, std::string const& field, bool defaultValue) {
			auto const& val = json[field];
			if (val.isNull()) return defaultValue;
			return val.asBool().unwrapOr(defaultValue);
		}

	}

	float AnimEvent::rollFrameTime() const {
		if (frameTime.has_value()) {
			return *frameTime;
		}
		if (minFrameTime.has_value() && maxFrameTime.has_value()) {
			static std::random_device rd;
			static std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dist(*minFrameTime, *maxFrameTime);
			return dist(gen);
		}
		// fallback, shouldnt happen. ever. i hope.
		return 0.f;
	}

	Result<AnimEvent, std::string> AnimEvent::parse(std::string const& eventName, matjson::Value const& json) {
		if (!json.isObject()) {
			return Err(fmt::format("event '{}': expected an object", eventName));
		}

		AnimEvent ev;

		ev.singleFrame = optionalBool(json, "singleFrame", false);

		auto subfolderRes = requireString(json, "subfolder", eventName);
		if (subfolderRes.isErr()) return Err(subfolderRes.unwrapErr());
		ev.subfolder = subfolderRes.unwrap();

		if (ev.singleFrame) {
			auto spriteNameRes = requireString(json, "spriteName", eventName);
			if (spriteNameRes.isErr()) return Err(spriteNameRes.unwrapErr());
			ev.spriteName = spriteNameRes.unwrap();
			ev.frameCount = 1;
		} else {
			auto baseNameRes = requireString(json, "spriteBaseName", eventName);
			if (baseNameRes.isErr()) return Err(baseNameRes.unwrapErr());
			ev.spriteBaseName = baseNameRes.unwrap();

			auto frameCountRes = requireInt(json, "frameCount", eventName);
			if (frameCountRes.isErr()) return Err(frameCountRes.unwrapErr());
			ev.frameCount = frameCountRes.unwrap();
			if (ev.frameCount <= 0) {
				return Err(fmt::format("event '{}': frameCount must be greater than 0", eventName));
			}
		}

		auto frameTimeRes = optionalFloat(json, "frameTime", eventName);
		if (frameTimeRes.isErr()) return Err(frameTimeRes.unwrapErr());
		ev.frameTime = frameTimeRes.unwrap();

		auto minRes = optionalFloat(json, "minFrameTime", eventName);
		if (minRes.isErr()) return Err(minRes.unwrapErr());
		ev.minFrameTime = minRes.unwrap();

		auto maxRes = optionalFloat(json, "maxFrameTime", eventName);
		if (maxRes.isErr()) return Err(maxRes.unwrapErr());
		ev.maxFrameTime = maxRes.unwrap();

		bool hasFixed = ev.frameTime.has_value();
		bool hasRange = ev.minFrameTime.has_value() || ev.maxFrameTime.has_value();

		if (hasFixed && hasRange) {
			return Err(fmt::format(
				"event '{}': specify either frameTime or minFrameTime/maxFrameTime, not both", eventName
			));
		}
		if (hasRange && (!ev.minFrameTime.has_value() || !ev.maxFrameTime.has_value())) {
			return Err(fmt::format(
				"event '{}': minFrameTime and maxFrameTime must both be set together", eventName
			));
		}
		if (!ev.singleFrame && !hasFixed && !hasRange) {
			return Err(fmt::format(
				"event '{}': must specify either frameTime or minFrameTime + maxFrameTime", eventName
			));
		}

		ev.loopAnim = optionalBool(json, "loopAnim", true);
		ev.lockRotation = optionalBool(json, "lockRotation", false);
		ev.canBeInterrupted = optionalBool(json, "canBeInterrupted", true);
		ev.interruptBySelf = optionalBool(json, "interruptBySelf", false);
		ev.keepPlayer = optionalBool(json, "keepPlayer", false);

		auto const& skipForLoopVal = json["skipForLoop"];
		if (!skipForLoopVal.isNull()) {
			if (!skipForLoopVal.isArray()) {
				return Err(fmt::format("event '{}': 'skipForLoop' must be an array of frame numbers", eventName));
			}
			if (ev.singleFrame || !ev.loopAnim) {
				geode::log::warn("event '{}': 'skipForLoop' does nothing on an animation that doesn't loop", eventName);
			}
			for (auto const& entry : skipForLoopVal) {
				auto intRes = entry.asInt();
				if (intRes.isErr()) {
					return Err(fmt::format("event '{}': 'skipForLoop' entries must all be whole numbers", eventName));
				}
				int frameNumber = static_cast<int>(intRes.unwrap());
				if (!ev.singleFrame && (frameNumber < 1 || frameNumber > ev.frameCount)) {
					return Err(fmt::format(
						"event '{}': 'skipForLoop' entry {} is out of range, frames go from 1 to {}",
						eventName, frameNumber, ev.frameCount
					));
				}
				if (std::find(ev.skipForLoop.begin(), ev.skipForLoop.end(), frameNumber) == ev.skipForLoop.end()) {
					ev.skipForLoop.push_back(frameNumber);
				}
			}
			if (!ev.singleFrame && static_cast<int>(ev.skipForLoop.size()) >= ev.frameCount) {
				return Err(fmt::format("event '{}': 'skipForLoop' skips every frame, at least one has to be left to loop on", eventName));
			}
		}

		auto holdForRes = optionalFloat(json, "holdFor", eventName);
		if (holdForRes.isErr()) return Err(holdForRes.unwrapErr());
		ev.holdFor = holdForRes.unwrap();

		auto const& cancelOnVal = json["cancelOn"];
		if (!cancelOnVal.isNull()) {
			if (!cancelOnVal.isArray()) {
				return Err(fmt::format("event '{}': 'cancelOn' must be an array of strings", eventName));
			}
			for (auto const& entry : cancelOnVal) {
				auto strRes = entry.asString();
				if (strRes.isErr()) {
					return Err(fmt::format("event '{}': 'cancelOn' entries must all be strings", eventName));
				}
				ev.cancelOn.push_back(strRes.unwrap());
			}
		}

		auto offsetXRes = optionalFloat(json, "offsetX", eventName);
		if (offsetXRes.isErr()) return Err(offsetXRes.unwrapErr());
		ev.offsetX = offsetXRes.unwrap().value_or(0.f);

		auto offsetYRes = optionalFloat(json, "offsetY", eventName);
		if (offsetYRes.isErr()) return Err(offsetYRes.unwrapErr());
		ev.offsetY = offsetYRes.unwrap().value_or(0.f);

		auto const& triggerOnVal = json["triggerOn"];
		if (!triggerOnVal.isNull()) {
			if (!triggerOnVal.isArray()) {
				return Err(fmt::format("event '{}': 'triggerOn' must be an array of strings", eventName));
			}
			for (auto const& entry : triggerOnVal) {
				auto strRes = entry.asString();
				if (strRes.isErr()) {
					return Err(fmt::format("event '{}': 'triggerOn' entries must all be strings", eventName));
				}
				ev.triggerOn.push_back(strRes.unwrap());
			}
		}

		auto fadeOutRes = optionalFloat(json, "fadeOutTime", eventName);
		if (fadeOutRes.isErr()) return Err(fadeOutRes.unwrapErr());
		ev.fadeOutTime = fadeOutRes.unwrap();

		return Ok(ev);
	}

	Result<void, std::string> GlobalAttributes::applyFrom(matjson::Value const& json) {
		if (!json.isObject()) {
			return Err(std::string("spr-pack.json: 'globalAttributes' must be an object"));
		}

		auto scaleRes = optionalFloat(json, "scale", "globalAttributes");
		if (scaleRes.isErr()) return Err(scaleRes.unwrapErr());
		if (auto newScale = scaleRes.unwrap()) {
			if (*newScale <= 0.f) {
				return Err(std::string("globalAttributes: 'scale' must be greater than 0"));
			}
			scale = *newScale;
		}

		pixelMode = optionalBool(json, "pixelMode", pixelMode);
		keepRobotFire = optionalBool(json, "keepRobotFire", keepRobotFire);
		keepSwingFires = optionalBool(json, "keepSwingFires", keepSwingFires);
		keepDashFire = optionalBool(json, "keepDashFire", keepDashFire);
		noDeathEffects = optionalBool(json, "noDeathEffects", noDeathEffects);

		return Ok();
	}

	Result<SpritePackMeta, std::string> SpritePackMeta::parse(matjson::Value const& json) {
		SpritePackMeta meta;

		auto setNameRes = requireString(json, "setName", "spr-pack.json");
		if (setNameRes.isErr()) return Err(setNameRes.unwrapErr());
		meta.setName = setNameRes.unwrap();

		auto authorRes = requireString(json, "author", "spr-pack.json");
		if (authorRes.isErr()) return Err(authorRes.unwrapErr());
		meta.author = authorRes.unwrap();

		auto const& descriptionVal = json["description"];
		if (!descriptionVal.isNull()) {
			auto descriptionRes = descriptionVal.asString();
			if (descriptionRes.isErr()) {
				return Err(std::string("spr-pack.json: 'description' must be a string"));
			}
			meta.description = descriptionRes.unwrap();
		}

		auto const& creditsVal = json["credits"];
		if (!creditsVal.isNull()) {
			if (!creditsVal.isArray()) {
				return Err(std::string("spr-pack.json: 'credits' must be an array of strings"));
			}
			for (auto const& entry : creditsVal) {
				auto strRes = entry.asString();
				if (strRes.isErr()) {
					return Err(std::string("spr-pack.json: 'credits' entries must all be strings"));
				}
				meta.credits.push_back(strRes.unwrap());
			}
		}

		return Ok(meta);
	}

	SoundEntry const* SoundSet::entry(std::string const& gamemode, std::string const& id) const {
		auto gmIt = gamemodes.find(gamemode);
		if (gmIt == gamemodes.end()) return nullptr;
		auto entryIt = gmIt->second.find(id);
		if (entryIt == gmIt->second.end()) return nullptr;
		return &entryIt->second;
	}

	std::vector<SoundEntry const*> SoundSet::matching(std::string const& gamemode, std::string const& eventName) const {
		std::vector<SoundEntry const*> result;

		auto gmIndexIt = index.find(gamemode);
		if (gmIndexIt == index.end()) return result;
		auto eventIt = gmIndexIt->second.find(eventName);
		if (eventIt == gmIndexIt->second.end()) return result;

		for (auto const& id : eventIt->second) {
			if (auto* found = this->entry(gamemode, id)) result.push_back(found);
		}
		return result;
	}

	Result<SoundSet, std::string> SoundSet::parse(fs::path const& packRoot, matjson::Value const& json) {
		if (!json.isObject()) {
			return Err(std::string("sounds.json: root must be an object keyed by gamemode"));
		}

		auto const normalRoot = packRoot.lexically_normal();
		SoundSet set;

		for (auto const& [gamemodeName, gamemodeJson] : json) {
			if (!gamemodeJson.isObject()) {
				return Err(fmt::format("sounds.json: gamemode '{}' must be an object of sound entries", gamemodeName));
			}

			for (auto const& [entryId, entryJson] : gamemodeJson) {
				auto ctx = fmt::format("sounds.json: {}/{}", gamemodeName, entryId);

				SoundEntry entry;
				entry.id = entryId;
				std::vector<std::string> relPaths;

				if (entryJson.isObject()) {
					// full form
					auto soundsRes = stringList(entryJson["sounds"], ctx + " 'sounds'");
					if (soundsRes.isErr()) return Err(soundsRes.unwrapErr());
					relPaths = soundsRes.unwrap();

					auto const& triggerVal = entryJson["triggerOn"];
					if (!triggerVal.isNull()) {
						auto triggerRes = stringList(triggerVal, ctx + " 'triggerOn'");
						if (triggerRes.isErr()) return Err(triggerRes.unwrapErr());
						entry.triggerOn = triggerRes.unwrap();
					}

					entry.ordered = optionalBool(entryJson, "ordered", false);
					entry.defaultEnabled = optionalBool(entryJson, "enabledByDefault", true);

					auto resetRes = optionalFloat(entryJson, "resetAfter", ctx);
					if (resetRes.isErr()) return Err(resetRes.unwrapErr());
					if (auto resetAfter = resetRes.unwrap()) {
						if (*resetAfter < 0.f) {
							return Err(fmt::format("{}: 'resetAfter' can't be negative", ctx));
						}
						if (!entry.ordered) {
							geode::log::warn("{}: 'resetAfter' does nothing without 'ordered'", ctx);
						}
						entry.resetAfter = *resetAfter;
					}
				} else {
					auto filesRes = stringList(entryJson, ctx);
					if (filesRes.isErr()) return Err(filesRes.unwrapErr());
					relPaths = filesRes.unwrap();
				}

				if (relPaths.empty()) {
					return Err(fmt::format("{}: needs at least one sound file", ctx));
				}
				if (entry.triggerOn.empty()) {
					entry.triggerOn.push_back(entryId);
				}

				for (auto const& rel : relPaths) {
					auto fileRes = resolveSoundFile(normalRoot, rel, ctx);
					if (fileRes.isErr()) return Err(fileRes.unwrapErr());
					if (auto file = fileRes.unwrap()) {
						entry.files.push_back(std::move(*file));
					}
				}

				if (entry.files.empty()) {
					geode::log::warn("{}: none of its files exist, ignoring the entry", ctx);
					continue;
				}
				set.gamemodes[gamemodeName][entryId] = std::move(entry);
			}
		}

		for (auto const& [gamemodeName, entries] : set.gamemodes) {
			auto& gmIndex = set.index[gamemodeName];
			for (auto const& [id, entry] : entries) {
				for (auto const& trigger : entry.triggerOn) {
					auto addFor = [&](std::string const& realEvent) {
						auto& ids = gmIndex[realEvent];
						if (std::find(ids.begin(), ids.end(), id) == ids.end()) ids.push_back(id);
					};

					if (auto* covered = shortcuts::expand(trigger)) {
						for (auto const& realEvent : *covered) addFor(realEvent);
					} else {
						addFor(trigger);
					}
				}
			}
		}

		return Ok(std::move(set));
	}

	AnimEvent const* SpritePack::findEvent(std::string const& gamemode, std::string const& triggerName) const {
		auto gmIt = gamemodes.find(gamemode);
		if (gmIt == gamemodes.end()) return nullptr;
		auto triggerIt = gmIt->second.triggerIndex.find(triggerName);
		if (triggerIt == gmIt->second.triggerIndex.end()) return nullptr;
		auto evIt = gmIt->second.events.find(triggerIt->second);
		if (evIt == gmIt->second.events.end()) return nullptr;
		return &evIt->second;
	}

	AnimEvent const* SpritePack::findEventById(std::string const& gamemode, std::string const& id) const {
		auto gmIt = gamemodes.find(gamemode);
		if (gmIt == gamemodes.end()) return nullptr;
		auto evIt = gmIt->second.events.find(id);
		if (evIt == gmIt->second.events.end()) return nullptr;
		return &evIt->second;
	}

	Result<SpritePack, std::string> SpritePack::parse(std::string const& id, fs::path const& rootPath, matjson::Value const& json) {
		if (!json.isObject()) {
			return Err(std::string("spr-pack.json: root must be an object"));
		}

		auto metaRes = SpritePackMeta::parse(json);
		if (metaRes.isErr()) return Err(metaRes.unwrapErr());

		SpritePack pack;
		pack.id = id;
		pack.rootPath = rootPath;
		pack.meta = metaRes.unwrap();

		auto const& globalAttributesVal = json["globalAttributes"];
		if (!globalAttributesVal.isNull()) {
			auto attrRes = pack.globalAttributes.applyFrom(globalAttributesVal);
			if (attrRes.isErr()) return Err(attrRes.unwrapErr());
		}

		auto const& eventsVal = json["events"];
		if (eventsVal.isNull()) {
			return Ok(pack);
		}
		if (!eventsVal.isArray()) {
			return Err(std::string("spr-pack.json: 'events' must be an array"));
		}

		// "events" is an array of objects, each keyed by gamemode name.
		// if the same gamemode/event pair shows up twice across entries, the later one takes prio
		for (auto const& block : eventsVal) {
			if (!block.isObject()) {
				return Err(std::string("spr-pack.json: each entry in 'events' must be an object keyed by gamemode"));
			}
			for (auto const& [gamemodeName, gamemodeJson] : block) {
				if (gamemodeName == "globalAttributes") {
					return Err(std::string("spr-pack.json: 'globalAttributes' goes at the root of the file, not inside 'events'"));
				}

				if (!gamemodeJson.isObject()) {
					return Err(fmt::format("gamemode '{}': expected an object of events", gamemodeName));
				}
				auto& gmEvents = pack.gamemodes[gamemodeName];
				for (auto const& [eventName, eventJson] : gamemodeJson) {
					auto evRes = AnimEvent::parse(fmt::format("{}/{}", gamemodeName, eventName), eventJson);
					if (evRes.isErr()) return Err(evRes.unwrapErr());
					auto ev = evRes.unwrap();
					ev.id = eventName;

					std::vector<std::string> effectiveTriggers =
						ev.triggerOn.empty() ? std::vector<std::string>{ ev.id } : ev.triggerOn;
					if (ev.fadeOutTime.has_value()) {
						bool isStaticDeathDisappear = false;
						for (auto const& t : effectiveTriggers) {
							if (t == "Mod:StaticDeathDisappear") { isStaticDeathDisappear = true; break; }
						}
						if (!isStaticDeathDisappear) {
							return Err(fmt::format(
								"event '{}/{}': 'fadeOutTime' is only valid on an event that triggers on "
								"Mod:StaticDeathDisappear",
								gamemodeName, eventName
							));
						}
					}

					gmEvents.events[eventName] = std::move(ev);
				}
			}
		}

		for (auto& [gamemodeName, gmEvents] : pack.gamemodes) {
			for (auto const& [eventId, ev] : gmEvents.events) {
				auto triggers = ev.triggerOn.empty() ? std::vector<std::string>{ eventId } : ev.triggerOn;
				for (auto const& trigger : triggers) {
					if (shortcuts::expand(trigger)) continue;
					gmEvents.triggerIndex[trigger] = eventId;
				}
			}

			for (auto const& [eventId, ev] : gmEvents.events) {
				auto triggers = ev.triggerOn.empty() ? std::vector<std::string>{ eventId } : ev.triggerOn;
				for (auto const& trigger : triggers) {
					if (auto* covered = shortcuts::expand(trigger)) {
						for (auto const& realEvent : *covered) {
							gmEvents.triggerIndex.emplace(realEvent, eventId);
						}
					}
				}
			}
		}

		return Ok(pack);
	}

}
