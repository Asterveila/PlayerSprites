#pragma once

#include <Geode/Result.hpp>
#include <matjson.hpp>

// hi ! if you're here before the mod is out for real...
// this is your wiki.
// im not writing one until ts is perfect.
// which it is not.
// good fucking luck.
namespace playersprites {

	namespace fs = std::filesystem;

	struct AnimEvent {
		std::string id;

		// array, contains all GJGameEvent names that trigger this animation.
		// if empty, attempts to use the id itself, expecting it to be a GameEvent name.
		// (i'd still advice to use this though)
		std::vector<std::string> triggerOn;

		// if singleFrame is true: spriteName is used instead of spriteBaseName (since there's no need to follow a pattern...)
		// frameCount is forced to 1, and frame time keys are not required (kind of obvious why). 
		// holdFor still controls how long the sprite stays active.
		bool singleFrame = false;
		std::string spriteName; // only used when singleFrame is true

		std::string subfolder;
		std::string spriteBaseName;
		int frameCount = 0;

		// at least one of these must be specified, unless singleFrame is true.
		std::optional<float> frameTime;
		std::optional<float> minFrameTime;
		std::optional<float> maxFrameTime;

		// self explanatory
		bool loopAnim = true;
		bool lockRotation = false;

		// how long the animation exists for. 
		// if -1, stays active until something else interrupts it. (mostly meant for idle platformer sprites)
		std::optional<float> holdFor;

		// if true, the vanilla GD icon/vehicle sprites stay visible alongside this custom sprite instead of being hidden. defaults to false.
		// normally the custom sprite fully replaces the vanilla icons. u can use this to make overlays or something.
		bool keepPlayer = false;

		// self explanatory. array of GameEvent names that can cancel this animation.
		// this is very useful for fine control over when animations stop if you're having issues
		std::vector<std::string> cancelOn;

		// self explanatory. right?
		// do note this is in cocos units, so it's not like plist sprite offsets.
		// moving 1 up here will be a lot more noticeable than moving 1 up in a plist file.
		float offsetX = 0.f;
		float offsetY = 0.f;

		// specific to Mod:StaticDeathDisappear.
		// how long the fade out anim takes when your animation's last frame is hit.
		std::optional<float> fadeOutTime;

		// self explanatory. if false, the animation will play out fully even if other events happen while this one's running.
		// if true, i sure fucking wonder.
		bool canBeInterrupted = true;

		// if true, this animation gets restarted from frame 0 when one of its own triggerOn events fires again while it's playing.
		// does NOT affect what OTHER events can do to it, that's still up to canBeInterrupted.
		bool interruptBySelf = false;

		float rollFrameTime() const;

		static geode::Result<AnimEvent, std::string> parse(std::string const& eventName, matjson::Value const& json);
	};
	struct GamemodeEvents {
		std::unordered_map<std::string, AnimEvent> events;
		std::unordered_map<std::string, std::string> triggerIndex;
	};

	struct SpritePackMeta {
		std::string setName;
		std::string author;
		std::vector<std::string> credits;

		static geode::Result<SpritePackMeta, std::string> parse(matjson::Value const& json);
	};

	struct SpritePack {
		// folder name, also used as internal id
		std::string id;
		fs::path rootPath;

		SpritePackMeta meta;

		std::unordered_map<std::string, GamemodeEvents> gamemodes;

		AnimEvent const* findEvent(std::string const& gamemode, std::string const& triggerName) const;
		AnimEvent const* findEventById(std::string const& gamemode, std::string const& id) const;

		static geode::Result<SpritePack, std::string> parse(
			std::string const& id, fs::path const& rootPath, matjson::Value const& json
		);
	};

}
