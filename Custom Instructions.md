# How to Define Custom Kuppa Script Instructions

To define a custom instruction, you need to define its *interface function* inside of the `ExtendedScriptCompiler` class in [extended_ks.h](source/extended_ks.h), and its *implementation function* in [extended_ks.impl](source/extended_ks.impl). The code in these files needs to inserted with NSMBe to make the new instruction work when called from a Kuppa Script, even if the script itself is defined in a DL or somewhere else.

## The two types of custom instructions

Each custom instruction defined using this system must be either a *player instruction* or a *camera instruction*. (See [this](<Custom Instructions.md#why-only-player-and-camera-instructions>) for why that is.) However, that doesn't mean that every new instruction has to deal with a player (i.e. a playable character) or the camera. Both player and camera instructions can ignore the player/camera they're given as a parameter and do anything the programmer wants, but I recommend using camera instructions for anything that doesn't involve a player to avoid spawning one by accident.

In the vanilla game, each player instruction must always specify the character it affects. If that character doesn't already exist in the level, it's spawned in for the duration of the cutscene. This is a problem for cutscenes that just want to do something with the current player character, regardless of who that happens to be. To solve this issue, [extended_ks.cpp](source/extended_ks.cpp) includes a patch that allows player instructions to leave the character unspecified, in which case the instruction will affect the character currently being played as. This even works with vanilla instructions, but [extended_ks.h](source/extended_ks.h) must be included to enable it.

## Interface functions

Here's an example of what an interface function in the `ExtendedScriptCompiler` class might look like:
```cpp
consteval auto SetCamAngleZ(short zAngle)
{
	return CamInstruction<46>(zAngle);
}
```

Because Kuppa Scripts are compiled at compile time, all interface functions must be declared with the `consteval` keyword. The `auto` keyword after that indicates that the return type of the function should be deduced based on whatever it returns. This instruction has a single parameter called `zAngle`, which represents the new angle of the camera along its z-axis. It's expected to be in binary angle units, where 0x4000 is equivalent to 90 degrees. The type of the parameter is `short` because 16 bits are enough to represent all possible angles in these units.

Because this interface function is for a camera instruction, it calls the `CamInstruction` function template and returns the object it returns. The number 46 inside the angle brackets is the *camera instruction ID* of this instruction, and it's used by the system to call the right implementation function. The vanilla game has 39 camera instructions, so the ID for all custom camera instructions must be at least 39.

The interface function of a player instruction looks similar, but slightly different:
```cpp
template<CharID Char = Any>
consteval auto SetPlayerPos(short x, short y, short z)
{
	return PlayerInstruction<Char, 14>(x, y, z);
}
```

Unlike the previous one, this interface function is actually a function template since character (`Mario`, `Luigi`, `Wario` or `Yoshi`) is specified as a template argument. The default argument is `Any`, which leaves the character unspecified as described above. The coordinates are 16-bit integers, which is enough to represent any position an object can be given in SM64DSe. To convert the coordinates in SM64DSe to these ones, divide them by 1000 or simply remove the decimal point.

Just like the interface function for the camera instruction called `CamInstruction` and returned its value, this one calls `PlayerInstruction` and returns its value. Along with the *player instruction ID*, the specified (or unspecified) character is given to it as a template argument.

Both `PlayerInstruction` and `CamInstruction` can be called with any number of function arguments of any number of types, as long as those types can be converted into byte arrays using [`std::bit_cast`](https://en.cppreference.com/w/cpp/numeric/bit_cast).

## Implementation functions

[todo]

## Why only player and camera instructions?

Most Kuppa Script instructions in the vanilla game can be divided into three categories based on the actor they deal with: player, camera and object. There are also instructions like `ChangeLevel` and `ChangeMusic` that don't deal with any actor. Each instruction has an instruction ID, which is determined the following way in each category:

 * Player instructions: 0-3, depending on the character
 * Camera instructions: always 4
 * Object instructions: 18-47, depending on how the object is supposed appear
 * Other instructions: 5-17, depending on what the instruction does

Player, camera and object instructions also have another ID, which is used as an index to an array of member function pointers to call the correct function. However, the main instruction ID isn't used as an index to anything. The code that checks it is like a `switch`-statement, and any instruction ID higher than 17 will spawn a cutscene object. (If the ID is higher than 47, the object will be "empty", but it could still be used for certain things.) This system for custom instructions is meant to work in all hacks, including those that feature cutscenes from the vanilla game, so it's better to use as few of these instruction IDs possible. Adding "other" instructions as their own category would also make this system even more complicated than it already is.

Unfortunately, the object instructions can't be used to manipulate all objects in the game. They only deal with a specific "cutscene object", which can take the appearance of a set number of actual objects based on the instruction ID. They are very specific to the cutscenes in the vanilla game, to the point where it wouldn't really make sense to write custom instructions for them. Even when dealing with objects whose appearance they can take, I'd recommend writing custom instructions to work with real instances of those objects instead of their cutscene object clones.

This leaves us with two catergories for custom instructions: player instructions for controlling playable characters, and "camera" instructions for anything else.
