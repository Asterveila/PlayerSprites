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
		bool lockRotation = false; // okay this might not be AS self explanatory. if true, disables player rotation.

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

		// list of frame numbers to skip when an animation loops (after the first full loop, first loop plays ALL frames, then skips these for all later loops).
		// frame numbers are the same as the files, not array-like indexes. if you wanna skip the first frame, use 1, not 0.
		// requires loopAnim to be true.
		std::vector<int> skipForLoop;

		float rollFrameTime() const;

		static geode::Result<AnimEvent, std::string> parse(std::string const& eventName, matjson::Value const& json);
	};
	struct GamemodeEvents {
		std::unordered_map<std::string, AnimEvent> events;
		std::unordered_map<std::string, std::string> triggerIndex;
	};

	// pack-wide settings for stuff that defines the overall look of ur sprites, all of em.
	// specially useful if u dont wanna resize pixel art sprites manually, which is a pain in the ass. (<- did it for 2 of the built-in packs. im sorry.)
	struct GlobalAttributes {
		// yeag
		float scale = 1.f;

		// if true, textures get setAliasTexParameters (nearest neighbor scaling, like the pixel art objects in the editor).
		// if false well nothing changes.
		bool pixelMode = false;

		// self explanatory. controls for robot/swing fires as well as dash fire anim.
		bool keepRobotFire = false;
		bool keepSwingFires = false;
		bool keepDashFire = false;

		// if true, the vanilla death effects (explosion, particles) are forced off.
		bool noDeathEffects = false;

		geode::Result<void, std::string> applyFrom(matjson::Value const& json);
	};

	struct SpritePackMeta {
		std::string setName;
		std::string author;
		std::vector<std::string> credits;
		std::string description;

		static geode::Result<SpritePackMeta, std::string> parse(matjson::Value const& json);
	};

	// I USED THE GLOBALSOUNDS TO DESTROY THE GLOBAL SOUNDS. (rest in pipis poorly customizable ass mod)
	struct SoundEntry {
		std::string id;
		std::vector<std::string> triggerOn; // same as the one for anims, array of events that trigger this sound list.
		std::vector<fs::path> files; // list of sound files to play.
		bool ordered = false; // only works when multiple sounds exist on an array. if true, plays them in order instead of picking one at random.
		float resetAfter = 0.f; // only if ordered is true. defines how long until the next sound to play is reset back to the first one (say, if u play 1 -> 2 and dont play 3, after this time, you wont play 3 next time but rather 1 again.)
		bool defaultEnabled = true; // self explanatory.
	};

	struct SoundSet {
		std::unordered_map<std::string, std::unordered_map<std::string, SoundEntry>> gamemodes;

		std::unordered_map<std::string, std::unordered_map<std::string, std::vector<std::string>>> index;

		bool empty() const { return gamemodes.empty(); }

		SoundEntry const* entry(std::string const& gamemode, std::string const& id) const;
		std::vector<SoundEntry const*> matching(std::string const& gamemode, std::string const& eventName) const;

		static geode::Result<SoundSet, std::string> parse(fs::path const& packRoot, matjson::Value const& json);
	};

	struct SpritePack {
		// folder name, also used as internal id
		std::string id;
		fs::path rootPath;

		SpritePackMeta meta;
		GlobalAttributes globalAttributes;
		SoundSet sounds;

		std::unordered_map<std::string, GamemodeEvents> gamemodes;

		AnimEvent const* findEvent(std::string const& gamemode, std::string const& triggerName) const;
		AnimEvent const* findEventById(std::string const& gamemode, std::string const& id) const;

		static geode::Result<SpritePack, std::string> parse(
			std::string const& id, fs::path const& rootPath, matjson::Value const& json
		);
	};

}
