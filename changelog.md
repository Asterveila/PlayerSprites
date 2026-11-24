# v1.1.1
- <cg>Added</c> back <cs>iOS Support.</c>
- <cg>Added</c> a setting to control the taunt button's scale.
- <cj>Fixed</c> some issues with the Sonic Sprite packs' Ball animations stopping when they shouldn't.

# v1.1.0
*It's worth noting that 90% of these changes were made still in 2.208, however version wasn't bumped since there weren't any official public releases after v1.0. Thanks for releasing 2.209* **_THE SAME DAY_** *i released this mod officially. Very cool robtop.*

<cr>iOS isn't supported yet in this version due to technical limitations *(missing bindings more specifically)*. Support will be added back as soon as possible.</c>

- <cd>Ported</c> to geode v6 (2.209)
- <cg>Added</c> <cs>**Taunt animations!**</c> Press a (configurable) keybind in-game to run random animations from your currently active Sprite Pack!
- <cg>Added</c> <cs>**In-Game Pack Management!**</c> You can now reorder and delete packs from the mod's UI.
- <cg>Added</c> the <cs>"tintColour"</c> key to colour sprites in animations.
- <cg>Added</c> the <cs>"keepRider"</c> key, to disable hiding the rider cube on Ship/UFO/Jetpack animations.
- <cg>Added</c> a new <cs>"Mod:HitGround"</c> special event, as a workaround to the game's Landing events not being properly triggered if a jump perfectly aligned you with a platform you land on. (Packs still have to manually declare this on their triggerOn/cancelOn entries, though)
- <cl>Allowed usage</c> of <cs>"Mod:Update"</c> on SFX packs, along with an "interval" key to play sounds repeatedly while Mod:Update is active.
- <cj>Fixed</c> an issue that made the Ghost Trail not reset back to vanilla when a vanilla icon was visible.
- <cj>Fixed</c> an issue that made it so if two animations had the same trigger on one pack, disabling the later declared one would not allow the other one to be active.
- <cj>Fixed</c> an issue that would make "Mod:Death" SFXs not play as they were overriden by vanilla's death sfx.
- <cj>Fixed</c> an issue that would make ship/ufo/jetpack animations with lockRotation set to true not properly flip the Sprites, sometimes leaving you with a non-flipped sprite on flipped gravity.
- <cj>Fixed</c> various issues related to the keepDashFire global attribute for packs.
- <cj>Fixed</c> various issues on the built-in sprite packs, as well as added a way to auto-update them (from the mod itself when updates release, it won't download anything off of anywhere).

# v1.0.0
- Initial release!
