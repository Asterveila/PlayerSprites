# 1.1.0
*It's worth noting that 90% of these changes were made still in 2.208, however version wasn't bumped since there weren't any official public releases after v1.0. Thanks for releasing 2.209* **_THE SAME DAY_** *i released this mod officially. Very cool robtop.*

- Ported to geode v6 (2.209) (<- this is currently false info LMAO)
- Added **Taunt animations!** Press a (configurable) keybind in-game to run random animations from your currently active Sprite Pack!
- Added **In-Game Pack Management!** You can now reorder and delete packs from the mod's UI.
- Added the "tintColour" key to colour sprites in animations.
- Added the "keepRider" key, to disable hiding the rider cube on Ship/UFO/Jetpack animations.
- Allowed usage of "Mod:Update" on SFX packs, along with an "interval" key to play sounds repeatedly while Mod:Update is active.
- Added a new "Mod:HitGround" special event, as a workaround to the game's Landing events not being properly triggered if a jump perfectly aligned you with a platform you land on. (Packs still have to manually declare this on their triggerOn/cancelOn entries, though)
- Fixed an issue that made the Ghost Trail not reset back to vanilla when a vanilla icon was visible.
- Fixed an issue that made it so if two animations had the same trigger on one pack, disabling the later declared one would not allow the other one to be active.
- Fixed an issue that would make "Mod:Death" SFXs not play as they were overriden by vanilla's death sfx.
- Fixed an issue that would make ship/ufo/jetpack animations with lockRotation set to true not properly flip the Sprites, sometimes leaving you with a non-flipped sprite on flipped gravity.
- Fixed various issues related to the keepDashFire global attribute for packs.


# 1.0.0
- Initial release!
