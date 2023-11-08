# How to Define Custom Kuppa Script Instructions

To define a custom instruction, you need to define its *interface function* inside of the `ExtendedScriptCompiler` class in [extended_ks.h](source/extended_ks.h), and its *implementation function* in [extended_ks.impl](source/extended_ks.impl). The code in these files needs to inserted with NSMBe to make the new instruction work when called from a Kuppa Script, even if the script itself is defined in a DL or somewhere else.

## The two types of custom instructions

Each custom instruction defined using this system must be either a *player instruction* or a *camera instruction*. (See [this](#why-only-player-and-camera-instructions) for why that is.) However, that doesn't mean that every new instruction has to deal with a player (i.e. a playable character) or the camera. Both player and camera instructions can ignore the player/camera they're given as a parameter and do anything the programmer wants, but I recommend using camera instructions for anything that doesn't involve a player to avoid spawning one by accident.

In the vanilla game, each player instruction must always specify the character it affects. If that character doesn't already exist in the level, it's spawned in for the duration of the cutscene. This is a problem for cutscenes that just want to do something with the current player character, regardless of who that happens to be. To solve this issue, [extended_ks.cpp](source/extended_ks.cpp) includes a patch that allows player instructions to leave the character unspecified, which makes the instruction affect the character currently being played as. This even works with vanilla instructions, but [extended_ks.h](source/extended_ks.h) must be included to enable it.

## Interface functions

### For camera instructions

Here's an example of what an interface function in the `ExtendedScriptCompiler` class might look like:

```cpp
consteval auto SetCamAngleZ(short zAngle)
{
	return CamInstruction<46>(zAngle);
}
```

Because Kuppa Scripts are compiled at compile time (duh), all interface functions must be declared with the `consteval` keyword. The `auto` keyword after that indicates that the return type of the function should be deduced based on whatever it returns, and should be used in all interface functions. This instruction has a single parameter called `zAngle`, which represents the new angle of the camera along its z-axis. It's expected to be in binary angle units, where 0x4000 is equivalent to 90 degrees. The type of the parameter is `short` because 16 bits are enough to represent all possible angles in these units.

Since this interface function is for a camera instruction, it calls the `CamInstruction` function template and returns the object it returns. The number 46 inside the angle brackets is the *camera instruction ID* of this instruction, and it's used by the system to call the right implementation function. The vanilla game has 39 camera instructions, so the ID for all custom camera instructions must be at least 39.

Now that our `SetCamAngleZ` instruction has an interface function, it may be called like this in a C++-embedded Kuppa Script:

```cpp
constinit auto script =
	NewScript().
	SetCamAngleZ(60_deg) (100).
	End();
```

If implemented properly, it would set the z-angle of the camera to 60 degrees at frame 100. The `_deg`-suffix automatically converts the angle from degrees to binary angle units.

### For player instructions

The interface function of a player instruction looks similar, but slightly different:

```cpp
template<CharID Char = Any>
consteval auto SetPlayerPos(short x, short y, short z)
{
	return PlayerInstruction<Char, 14>(x, y, z);
}
```

Unlike the previous one, this interface function is actually a function template since the character (`Mario`, `Luigi`, `Wario`, `Yoshi` or `Any`) is given as a template argument. The default argument is `Any`, which leaves the character unspecified. The parameters are 16-bit integers, which is enough to represent any position an object can be given in SM64DSe. To convert the coordinates in SM64DSe to these ones, divide them by 1000 or simply remove the decimal point.

Just like the interface function for the camera instruction called `CamInstruction` and returned its value, this one calls `PlayerInstruction` and returns its value. Along with the *player instruction ID*, the specified (or unspecified) character `Char` is given to it as a template argument. Both `PlayerInstruction` and `CamInstruction` can be called with any number of function arguments of any number of types, as long as those types can be converted to byte arrays using [`std::bit_cast`](https://en.cppreference.com/w/cpp/numeric/bit_cast).

Our `SetPlayerPos` instruction could be called in a C++-embedded Kuppa Script in the following ways:

```cpp
constinit auto script =
	NewScript().
	SetPlayerPos       (-3593, 3662, -5462) (100). // Sets the position of the current player
	SetPlayerPos<Luigi>(-3030, 3095,  -905) (200). // Sets the position of Luigi specifically, and spawns him if necessary
	End();
```

### Default arguments

Like most functions in C++, instruction interface functions can use default arguments:

```cpp
consteval auto ExpDecayCamAngleZ(short targetAngle, uint16_t invFactor, uint16_t maxDelta = 180_deg, uint16_t minDelta = 0)
{
	return CamInstruction<48>(targetAngle, invFactor, maxDelta, minDelta);
}

template<CharID Char = Any> // They also work in function templates!
consteval auto ExpDecayPlayerAngleY(short targetAngle, uint16_t invFactor, uint16_t maxDelta = 180_deg, uint16_t minDelta = 0)
{
	return PlayerInstruction<Char, 18>(targetAngle, invFactor, maxDelta, minDelta);
}
```

Parameters with default arguments don't need to be given arguments when calling the instruction in a script:

```cpp
constinit auto script =
	NewScript().
	ExpDecayCamAngleZ(-90_deg, 10, 5_deg) (0, 99).   // maxDelta = 5_deg, minDelta = 0
	ExpDecayPlayerAngleY(90_deg, 20)      (100, -1). // maxDelta = 180_deg, minDelta = 0
	End();
```

## Implementation functions

### For camera instructions

The `SetCamAngleZ` instruction from the last section can be implemented by adding the following code to [extended_ks.impl](source/extended_ks.impl):
```cpp
IMPLEMENT(SetCamAngleZ)
(Camera& cam, const char* params, short minFrame, short maxFrame)
{
	cam.zAngle = ReadUnaligned<short>(params);
}
```
The `IMPLEMENT` macro specifies the name of the instruction being implemented, which is `SetCamAngleZ` in this case. That's followed by the parameter list of the implementation function, which should be the same for all camera instructions. It contains a reference to the camera, a pointer to a byte array, and the minimum and maximum frame numbers on which the instruction is run. The byte array pointed by `params` contains all parameters given to the instruction, packed right next to each other without any padding between them. In this case, however, there is only one parameter and it's of type `short`. It's read from the buffer using the `ReadUnaligned` function template from [unaligned.h](source/unaligned.h), which can be used to read parameters of (almost) any type. The parameter read from the buffer is assigned to the z-angle of the camera, like the name of the instruction suggests.

### For player instructions

Likewise, the `SetPlayerPos` instruction from the previous section can be implemented like this:
```cpp
IMPLEMENT(SetPlayerPos<>)
(Player& player, const char* params, short minFrame, short maxFrame)
{
	player.pos = ReadUnaligned<Vector3_16>(params);
}
```
Because the interface function is a function template, its name given to the `IMPLEMENT` macro needs to be followed by a template argument list in angle brackets. In this case the list is empty because the only template parameter has a default argument.

The function parameter list is the same as the one for camera instructions, except for the first parameter which is a reference to a player instead of the camera. Even though the parameters are declared as three `short`s in the interface function, they can be read as a `Vector3_16`, a struct of three `short`s. When the `ReadUnaligned` function template is used to read a `Vector3`, a `Vector3_16` or a `Vector3_16f`, its return type is a lazy-evaluated proxy for `Vector3`. (See `Vector3::Proxy` in [SM64DS_Common.h](source/include/SM64DS_Common.h).) With all other types, the return type would be the same as the template argument, but this specialization for vectors makes it easier to read them while avoiding unnecessary copies. The type of `player.pos` is `Vector3`, so the proxy returned by `ReadUnaligned<Vector3_16>` can be assigned to it directly.

### Reading multiple parameters

The `ExpDecayCamAngleZ` from the last section can be implemented using the `ApproachAngle` function from the vanilla game. One might do that like this:

```cpp
IMPLEMENT(ExpDecayCamAngleZ)
(Camera& cam, const char* params, short minFrame, short maxFrame)
{
	const short targetAngle  = ReadUnaligned<short>(params);
	const uint16_t invFactor = ReadUnaligned<short>(params + 2);
	const uint16_t maxDelta  = ReadUnaligned<short>(params + 4);
	const uint16_t minDelta  = ReadUnaligned<short>(params + 6);

	ApproachAngle(cam.zAngle, targetAngle, invFactor, maxDelta, minDelta);
}
```

However, this is not be the best approach. `ReadUnaligned` is called for each parameter separately, which requires offsetting the `params` pointer by the right amount for each of them. This can get confusing with multiple parameters, especially if their sizes differ. Fortunately, `ReadUnaligned` allows reading multiple parameters at once while calculating the offset for each of them automatically. Here's the same instruction implemented this way:

```cpp
IMPLEMENT(ExpDecayCamAngleZ)
(Camera& cam, const char* params, short minFrame, short maxFrame)
{
	const auto [targetAngle, invFactor, maxDelta, minDelta]
		= ReadUnaligned<short, uint16_t, uint16_t, uint16_t>(params);

	ApproachAngle(cam.zAngle, targetAngle, invFactor, maxDelta, minDelta);
}
```

When called with multiple template arguments, `ReadUnaligned` returns an instance of [`std::tuple`](https://en.cppreference.com/w/cpp/utility/tuple) containing the parameters it read. In most cases I'd recommend extracting them from the tuple to appropriately named local variables using a [structured binding](https://en.cppreference.com/w/cpp/language/structured_binding), like in the example above.

### Using helper functions

Although the file extension might look unfamiliar, the code in [extended_ks.impl](source/extended_ks.impl) is just regular C++ source code. This means that it's possible to use functions, classes, [asm-declarations](https://en.cppreference.com/w/cpp/language/asm), etc. along with the instruction implementations.

Let's say we also want to implement the `ExpDecayPlayerAngleY` instruction, which is very similar to `ExpDecayCamAngleZ`. On its own the implementation would look like this:

```cpp
IMPLEMENT(ExpDecayPlayerAngleY<>)
(Player& player, const char* params, short minFrame, short maxFrame)
{
	const auto [targetAngle, invFactor, maxDelta, minDelta]
		= ReadUnaligned<short, uint16_t, uint16_t, uint16_t>(params);

	ApproachAngle(player.ang.y, targetAngle, invFactor, maxDelta, minDelta);
}
```

We can see that it's indeed very similar to that of `ExpDecayCamAngleZ`. In fact, most of their implementations can be moved to a single function:

```cpp
static void CallApproachAngle(short& angle, const char* params)
{
	const auto [targetAngle, invFactor, maxDelta, minDelta]
		= ReadUnaligned<short, uint16_t, uint16_t, uint16_t>(params);

	ApproachAngle(angle, targetAngle, invFactor, maxDelta, minDelta);
}

IMPLEMENT(ExpDecayCamAngleZ)
(Camera& cam, const char* params, short minFrame, short maxFrame)
{
	CallApproachAngle(cam.zAngle, params);
}

IMPLEMENT(ExpDecayPlayerAngleY<>)
(Player& player, const char* params, short minFrame, short maxFrame)
{
	CallApproachAngle(player.ang.y, params);
}
```

This works just like before, but saves some space and avoids duplicating code.

### Overloading

Let's say I wanted to call `SetPlayerPos` in a script while using a variable for the position vector:

```cpp
constexpr Vector3_16 spawnPos = {-1200, 254, 6800};

constinit auto script =
	NewScript().
	SetPlayerPos(spawnPos) (0).
	End();
```

Because the interface function for `SetPlayerPos` takes three `short` parameters instead of a single `Vector3_16` parameter, this wouldn't work yet. To make it work, the interface function needs to be overloaded:

```cpp
template<CharID Char = Any>
consteval auto SetPlayerPos(Vector3_16 pos)
{
	return PlayerInstruction<Char, 14>(pos);
}

template<CharID Char = Any>
consteval auto SetPlayerPos(short x, short y, short z)
{
	return PlayerInstruction<Char, 14>(x, y, z);
}
```

This particular instruction uses the same ID in both overloads, but it's also possible to use a different one in each of them. In the latter case, each overload would call a different implementation function. Because of this, the implementation functions of all overloaded instructions need to be implemented a bit differently from others. For example, the implementation function for `SetPlayerPos` would need to be changed to this:

```cpp
IMPLEMENT_OVERLOAD(SetPlayerPos, Vector3_16)
(Player& player, const char* params, short minFrame, short maxFrame)
{
	player.pos = ReadUnaligned<Vector3_16>(params);
}
```

The `IMPLEMENT_OVERLOAD` macro is used instead of `IMPLEMENT`, and the name of the interface function is followed by the parameter types of the intended overload. Since both overloads use the same ID, they must both share the same implementation function. If both of them instead had their own instruction IDs, they would need to have two different implementation functions, one defined with `IMPLEMENT_OVERLOAD(SetPlayerPos, Vector3_16)` and another with `IMPLEMENT_OVERLOAD(SetPlayerPos, short, short, short)`.

### Implementation by ID

I don't have an example of where this would be particularly useful yet, but it's also possible to specify the ID directly in the implementation function instead of deducing it from an interface function. This can be done with the `IMPLEMENT_ID` macro:

```cpp
IMPLEMENT_ID(Player, 32) // Implements player instruction 32
(Player& player, const char* params, short minFrame, short maxFrame)
{
	// ...
}

IMPLEMENT_ID(Camera, 64) // Implements camera instruction 64
(Camera& cam, const char* params, short minFrame, short maxFrame)
{
	// ...
}
```

I wouldn't recommend this for most instructions since it requires updating the instruction ID in [extended_ks.impl](source/extended_ks.impl) every time it's changed in [extended_ks.h](source/extended_ks.h) and it's easy to forget that.

## Why only player and camera instructions?

Most Kuppa Script instructions in the vanilla game can be divided into three categories based on the actor they deal with: player, camera and object. There are also instructions like `ChangeLevel` and `ChangeMusic` that don't deal with any actor. Each instruction has an instruction ID, which is determined the following way in each category:

 * Player instructions: 0-3, depending on the character
 * Camera instructions: always 4
 * Object instructions: 18-47, depending on how the object is supposed appear
 * Other instructions: 5-17, depending on what the instruction does

Player, camera and object instructions also have another ID, which is used as an index to an array of member function pointers to call the correct function. However, the main instruction ID isn't used as an index to anything. The code that checks it is like a `switch`-statement, and any instruction ID higher than 17 will spawn a cutscene object. (If the ID is higher than 47, the object will be "empty", but it could still be used for certain things.) This system for custom instructions is meant to work in all hacks, including those that feature cutscenes from the vanilla game, so it's better to use as few of these instruction IDs possible. Adding "other" instructions as their own category would also make this system even more complicated than it already is.

Unfortunately, the object instructions can't be used to manipulate all objects in the game. They only deal with a specific "cutscene object", which can take the appearance of a set number of actual objects based on the instruction ID. They are very specific to the cutscenes in the vanilla game, to the point where it wouldn't really make sense to write custom instructions for them. Even when dealing with objects whose appearance they can take, I'd recommend writing custom instructions to work with real instances of those objects instead of their cutscene object clones.

This leaves us with two categories for custom instructions: player instructions for controlling playable characters, and "camera" instructions for anything else.
