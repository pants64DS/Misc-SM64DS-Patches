#ifndef SM64DS_CUTSCENE_INCLUDED
#define SM64DS_CUTSCENE_INCLUDED

#include "SM64DS_Common.h"
#include <cstdint>
#include <concepts>
#include <array>
#include <bit>

class Camera;

template<class T>
using KS_MemberFuncPtr = int(T::*)(char* params, short minFrame, short maxFrame);

extern "C"
{
	bool RunKuppaScript(char* address);
	void EndKuppaScript();

	// The array of player functions (resp. camera functions) is initialized when the first player
	// instruction (resp. camera instruction) is run. Functions in both arrays usually return 1.

	extern KS_MemberFuncPtr<Player> KS_PLAYER_FUNCTIONS[14];
	extern KS_MemberFuncPtr<Camera> KS_CAMERA_FUNCTIONS[39];

	extern unsigned KS_FRAME_COUNTER;
	extern char* RUNNING_KUPPA_SCRIPT; // nullptr if no script is running
}

template<std::size_t size> requires(size > 0)
struct KuppaScript
{
	std::array<char, size> data;

	consteval explicit KuppaScript(const decltype(data)& data):
		data(data)
	{}

	constexpr       char* Raw()       { return data.data(); }
	constexpr const char* Raw() const { return data.data(); }

	bool Run() & { return RunKuppaScript(data.data()); }

	static void Stop() { EndKuppaScript(); }
};

namespace KuppaScriptImpl {

template<uint8_t id>
struct CharID_Type
{
	static constexpr uint8_t instructionID = id;
};

template<class T>
concept CharID = std::same_as<T, CharID_Type<T::instructionID>>;

template<template<std::size_t> class>
struct DefaultCharImpl { using Type = void; };

template<template<std::size_t> class Compiler>
using DefaultChar = DefaultCharImpl<Compiler>::Type;

template<class T, std::size_t s1, std::size_t s2>
consteval auto operator+(const std::array<T, s1>& a1, const std::array<T, s2>& a2)
{
	std::array<T, s1 + s2> res;

	for (std::size_t i = 0; i < a1.size();++i)
		res[i] = a1[i];

	for (std::size_t i = 0; i < a2.size();++i)
		res[a1.size() + i] = a2[i];

	return res;
}

consteval auto ToByteArray(const auto&... args)
{
	if constexpr (sizeof...(args) == 0)
		return std::array<char, 0> {};
	else
		return (std::bit_cast<std::array<char, sizeof(args)>>(args) + ...);
}

template<template<std::size_t scriptSize> class DerivedCompiler, std::size_t scriptSize>
class BaseScriptCompiler
{
	const std::array<char, scriptSize> precedingScript;

public:
	template<std::size_t paramListSize>
	struct PendingInstruction
	{
		uint8_t id;
		std::array<char, scriptSize> precedingScript;
		std::array<char, paramListSize> params;

		static constexpr std::size_t size = 6 + paramListSize;
		static_assert(size < 256, "A Kuppa Script command is too long");

		consteval auto operator()(short minFrame, short maxFrame) const
		{
			return DerivedCompiler<scriptSize + size>
			{
				precedingScript
				+ std::to_array<char>({size, id})
				+ ToByteArray(minFrame, maxFrame)
				+ params
			};
		}

		consteval auto operator()(short frame) const
		{
			return operator()(frame, frame);
		}
	};

	consteval BaseScriptCompiler() requires(scriptSize == 0) = default;

	consteval BaseScriptCompiler(const decltype(precedingScript)& precedingScript) requires(scriptSize > 0):
		precedingScript(precedingScript)
	{}

	consteval auto End()
	{
		return KuppaScript<scriptSize + 1>{precedingScript + std::to_array<char>({0})};
	}

	template<uint8_t id, class... Args>
	consteval PendingInstruction<(sizeof(Args) + ... + 0)> Instruction(const Args&... args)
	{
		return {id, precedingScript, ToByteArray(args...)};
	}

	template<CharID Char, uint8_t subID, class... Args>
	consteval auto PlayerInstruction(const Args&... args)
	{
		return Instruction<Char::instructionID>(subID, args...);
	}

	template<uint8_t subID, class... Args>
	consteval auto CamInstruction(const Args&... args)
	{
		return Instruction<4>(subID, args...);
	}

	// Set camera target position (coordinates in fxu)
	consteval auto SetCamTarget(Vector3_16 target)
	{
		return CamInstruction<0>(target);
	}

	consteval auto SetCamTarget(short x, short y, short z)
	{
		return SetCamTarget(Vector3_16{x, y, z});
	}

	// Set camera position (coordinates in fxu)
	consteval auto SetCamPos(Vector3_16 pos)
	{
		return CamInstruction<1>(pos);
	}

	consteval auto SetCamPos(short x, short y, short z)
	{
		return SetCamPos(Vector3_16{x, y, z});
	}

	// Set camera target position and position (coordinates in fxu)
	consteval auto SetCamTargetAndPos(Vector3_16 target, Vector3_16 pos)
	{
		return CamInstruction<2>(target, pos);
	}

	consteval auto SetCamTargetAndPos(short tx, short ty, short tz, short px, short py, short pz)
	{
		return SetCamTargetAndPos(Vector3_16{tx, ty, tz}, Vector3_16{px, py, pz});
	}

	// Set camera FOV modifier (not sure if it's exactly the FOV, might be FOV/2)
	consteval auto SetCamFOV(uint16_t fovModifier)
	{
		return CamInstruction<3>(fovModifier);
	}

	// Adjust camera FOV modifier
	consteval auto AdjustCamFOV(uint16_t targetModifier, uint16_t speed)
	{
		return CamInstruction<4>(targetModifier, speed);
	}

	// Adjust screen size from full size to target values (uses minFrame and maxFrame for gradient)
	consteval auto AdjustCamScreenSize(uint8_t left, uint8_t bottom, uint8_t right, uint8_t top)
	{
		return CamInstruction<5>(left, bottom, right, top);
	}

	// Weird cubic interpolation?
	consteval auto UnkCubicInterpolation(unsigned unk0, unsigned unk1) // pointers?
	{
		return CamInstruction<6>(unk0, unk1);
	}

	// Set a stored value presumably used by the camera
	consteval auto SetStoredFix12(Fix12i newValue)
	{
		return CamInstruction<13>(newValue);
	}

	// The same as above but the type is an int for backwards compatibility
	consteval auto SetStoredFix12(int newValue)
	{
		return CamInstruction<13>(newValue);
	}

	consteval auto AdjustStoredFix12(Fix12i target, Fix12i speed)
	{
		return CamInstruction<14>(target, speed);
	}

	// The same as above but types are int for backwards compatibility
	consteval auto AdjustStoredFix12(int target, int speed)
	{
		return CamInstruction<14>(target, speed);
	}

	// Adjust camera target position via exponential decay
	consteval auto AdjustCamTargetDec(
		Vector3_16 dest,    // the destination vector of the target position
		Vector3_16 approach // approach factors
	)
	{
		return CamInstruction<15>(dest, approach);
	}

	consteval auto AdjustCamTargetDec(
		Vector3_16 dest,             // the destination vector of the target position
		short ax, short ay, short az // approach factors
	)
	{
		return AdjustCamTargetDec(dest, Vector3_16{ax, ay, az});
	}

	consteval auto AdjustCamTargetDec(
		Vector3_16 dest, // the destination vector of the target position
		short approach   // single approach factor
	)
	{
		return AdjustCamTargetDec(dest, approach, approach, approach);
	}

	consteval auto AdjustCamTargetDec(
		short dx, short dy, short dz, // the destination coordinates of the target position
		short ax, short ay, short az  // approach factors
	)
	{
		return AdjustCamTargetDec(Vector3_16{dx, dy, dz}, Vector3_16{ax, ay, az});
	}

	consteval auto AdjustCamTargetDec(
		short dx, short dy, short dz, // the destination coordinates of the target position
		short approach                // single approach factor
	)
	{
		return AdjustCamTargetDec(Vector3_16{dx, dy, dz}, approach);
	}

	// Adjust camera position via exponential decay
	consteval auto AdjustCamPosDec(
		Vector3_16 dest,    // the destination vector of the target position
		Vector3_16 approach // approach factors
	)
	{
		return CamInstruction<16>(dest, approach);
	}

	consteval auto AdjustCamPosDec(
		Vector3_16 dest,             // the destination vector of the target position
		short ax, short ay, short az // approach factors
	)
	{
		return AdjustCamPosDec(dest, Vector3_16{ax, ay, az});
	}

	consteval auto AdjustCamPosDec(
		Vector3_16 dest, // the destination vector of the target position
		short approach   // single approach factor
	)
	{
		return AdjustCamPosDec(dest, approach, approach, approach);
	}

	consteval auto AdjustCamPosDec(
		short dx, short dy, short dz, // the destination coordinates of the position
		short ax, short ay, short az  // approach factors
	)
	{
		return AdjustCamPosDec(Vector3_16{dx, dy, dz}, Vector3_16{ax, ay, az});
	}

	consteval auto AdjustCamPosDec(
		short dx, short dy, short dz, // the destination coordinates of the position
		short approach                // single approach factor
	)
	{
		return AdjustCamPosDec(Vector3_16{dx, dy, dz}, approach);
	}

	// Set stored angle toward pause view position and the stored shorts to 0
	consteval auto ResetCamToPause()
	{
		return CamInstruction<17>();
	}

	// Adjust camera target position to offset from owner via exponential decay
	consteval auto SetCamTargetRelativeDec(Vector3_16 offset, uint8_t approachFactorLsl8)
	{
		return CamInstruction<18>(offset, approachFactorLsl8);
	}

	consteval auto SetCamTargetRelativeDec(short x, short y, short z, uint8_t approachFactorLsl8)
	{
		return SetCamTargetRelativeDec(Vector3_16{x, y, z}, approachFactorLsl8);
	}

	// Adjust camera target position to offset from owner rotated by owner's facing angle via exponential decay
	consteval auto AdjustCamByOwnerAngleDec(Vector3_16 offset, uint8_t approachFactorLsl8)
	{
		return CamInstruction<19>(offset, approachFactorLsl8);
	}

	consteval auto AdjustCamByOwnerAngleDec(short x, short y, short z, uint8_t approachFactorLsl8)
	{
		return AdjustCamByOwnerAngleDec(Vector3_16{x, y, z}, approachFactorLsl8);
	}

	// Adjust camera position and angles relative to owner position via exponential decay
	consteval auto CamPosAngleRelativeDec(
		uint16_t targetDist,           // target distance (in fxu)
		uint8_t  approachFactorLsl8,   // approach factor multiplied by 256
		short    targetVertAngle,      // target vertical angle
		uint8_t  invVertAngleApproach, // 1 divided by the vertical angle's approach factor (Set to 0 to not change vertical angle)
		short    targetHorzAngle,      // Target horizontal angle (Set to -1 to use CAMERA->targetAngle),
		uint8_t  invHorzAngleApproach  // 1 divided by the horizontal angle's approach factor (Set to 0 to not change horizontal angle)
	)
	{
		return CamInstruction<20>(
			targetDist, approachFactorLsl8,
			targetVertAngle, invVertAngleApproach,
			targetHorzAngle, invHorzAngleApproach
		);
	}

	// Spin camera target position around camera position
	// Sets a stored vector to the target position on the first frame
	consteval auto SpinCamTarget(short vertAngularSpeed, short horzAngularSpeed)
	{
		return CamInstruction<21>(vertAngularSpeed, horzAngularSpeed);
	}

	// Spin the camera position around its owner position
	consteval auto SpinCamAroundOwnerPos(int speed, short vertAngularSpeed, short horzAngularSpeed)
	{
		return CamInstruction<22>(speed, vertAngularSpeed, horzAngularSpeed);
	}

	// Adjust FOV modifier via exponential decay (invApproachFactor = 1 / approachFactor)
	consteval auto AdjustCamFOVDec(uint16_t targetModifier, uint8_t invApproachFactor)
	{
		return CamInstruction<23>(targetModifier, invApproachFactor);
	}

	// Adjust camera FOV if new value is greater than the old value
	consteval auto AdjustCamFOVIfBigger(
		uint16_t newModifier,
		short    fovSpeedTowards0,
		short    fovOscillationAngularSpeed
	)
	{
		return CamInstruction<24>(
			newModifier,
			fovSpeedTowards0,
			fovOscillationAngularSpeed
		);
	}

	// Not actually a nop, but something to do with angles
	consteval auto NopCam()
	{
		return CamInstruction<25>();
	}

	// Approach a distance of 800 fxu (approach factor = 1/20),
	// 33.75° above the player (approach factor = 1/16), with the
	// horizontal angle being the stored angle (approach factor = 1/8)
	consteval auto CamApproachPlayerFromTop()
	{
		return CamInstruction<26>();
	}

	// Set camera target position and position to rotated offset from owner (in fxu)
	consteval auto SetCamTargetAndPosRotatedFromOwner(Vector3_16 target, Vector3_16 pos)
	{
		return CamInstruction<27>(target, pos);
	}

	consteval auto SetCamTargetAndPosRotatedFromOwner(short tx, short ty, short tz, short px, short py, short pz)
	{
		return SetCamTargetAndPosRotatedFromOwner(Vector3_16{tx, ty, tz}, Vector3_16{px, py, pz});
	}

	// Waits for a button or touchscreen input and returns to the title screen (previously named "Wifi" for some reason)
	consteval auto WaitAndLoadTitleScreen() { return Instruction<5>(); }

	// Change cutscene script (by raw address)
	consteval auto ChangeScript(unsigned newScriptAddress)
	{
		return Instruction<6>(newScriptAddress);
	}

	// Change music
	consteval auto ChangeMusic(unsigned musicID)
	{
		return Instruction<7>(musicID);
	}

	// Play sound from SSAR 1 (player voices). [].
	consteval auto PlaySoundSSAR1(unsigned soundID)
	{
		return Instruction<8>(soundID);
	}
	
	// Play sound from SSAR 2 (system)
	consteval auto PlaySoundSSAR2(unsigned soundID)
	{
		return Instruction<9>(soundID);
	}
	
	// Display a message
	consteval auto DisplayMessage(uint16_t messageID)
	{
		return Instruction<10>(messageID);
	}

	// Change level (with raw cutscene address)
	consteval auto ChangeLevel(
		uint8_t  newLevelID,
		uint8_t  entranceID,
		uint8_t  starID,
		unsigned cutsceneAddress = 0 // 0 if none
	)
	{
		return Instruction<11>(
			newLevelID, entranceID, starID,
			cutsceneAddress
		);
	}

	// Fade to white
	consteval auto FadeToWhite() { return Instruction<12>(); }
	
	// Fade from white
	consteval auto FadeFromWhite() { return Instruction<13>(); }
	
	// Fade to black
	consteval auto FadeToBlack() { return Instruction<14>(); }
	
	// Fade from white, but broken
	consteval auto FadeFromWhiteBroken() { return Instruction<15>(); }
	
	// Fade to black, then fade from black
	consteval auto FadeToBlackAndBack() { return Instruction<16>(); }
	
	// Enables the "Ambient Sound Effects" objects by storing 0 at 0x02110aec (previously named "STZ")
	consteval auto EnableAmbientSoundEffects() { return Instruction<17>(); }

	// Set position and Y-angle (both model and motion angle)
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SetPlayerPosAndAngleY(Vector3_16 pos, short angleY)
	{
		return PlayerInstruction<Char, 0>(pos, angleY);
	}

	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SetPlayerPosAndAngleY(short xPos, short yPos, short zPos, short angleY)
	{
		return SetPlayerPosAndAngleY<Char>(Vector3_16{xPos, yPos, zPos}, angleY);
	}

	// Send input to move the player to a target position (full magnitude: 0x1000)
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SendPlayerInput(Vector3_16 pos, short inputMagnitude)
	{
		return PlayerInstruction<Char, 1>(pos, inputMagnitude);
	}

	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SendPlayerInput(short xPos, short yPos, short zPos, short inputMagnitude)
	{
		return SendPlayerInput<Char>(Vector3_16{xPos, yPos, zPos}, inputMagnitude);
	}

	// Send input to move the player to a target position (full magnitude: 1._fs)
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SendPlayerInput(Vector3_16 pos, Fix12s inputMagnitude = 1._fs)
	{
		return PlayerInstruction<Char, 1>(pos, inputMagnitude);
	}

	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto SendPlayerInput(short xPos, short yPos, short zPos, Fix12s inputMagnitude = 1._fs)
	{
		return SendPlayerInput<Char>(Vector3_16{xPos, yPos, zPos}, inputMagnitude);
	}

	// Orr player flags with 0x24000000 (previously named OrrPlayerFlags)
	template<class Char = DefaultChar<DerivedCompiler>>
	consteval auto ActivatePlayer()
	{
		return PlayerInstruction<Char, 2>();
	}

	// Make a player lie down
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto MakePlayerLieDown()
	{
		return PlayerInstruction<Char, 3>();
	}

	// Player character voice
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto PlayPlayerVoice(unsigned soundID)
	{
		return PlayerInstruction<Char, 4>(soundID);
	}

	// Play sound from SSAR0
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto PlayerPlaySoundSSAR0(unsigned soundID)
	{
		return PlayerInstruction<Char, 5>(soundID);
	}

	// Play sound from SSAR3
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto PlayerPlaySoundSSAR3(unsigned soundID)
	{
		return PlayerInstruction<Char, 6>(soundID);
	}

	// Press and hold buttons
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto PlayerHoldButtons(uint16_t buttons)
	{
		return PlayerInstruction<Char, 7>(buttons);
	}

	// Drop the player with a speed of 32 fxu/frame and give him wings for 408 frames
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto GivePlayerWingsAndDrop()
	{
		return PlayerInstruction<Char, 8>();
	}

	// Hurt the player with an imaginary source 80 fxu away
	// If the player is Luigi, spawn ouch stars as well. [DirectionOfSource].
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto HurtPlayer(short directionOfSource)
	{
		return PlayerInstruction<Char, 9>(directionOfSource);
	}

	// A weird cap animation
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto AnimatePlayerCap(uint8_t state)
	{
		return PlayerInstruction<Char, 10>(state);
	}

	// Turn the player via exponential decay. [NewAngle].
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto TurnPlayerDec(short newAngleY)
	{
		return PlayerInstruction<Char, 11>(newAngleY);
	}

	// Make the player move forward at a certain speed (does not change animation)
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto MovePlayerForward()
	{
		return PlayerInstruction<Char, 12>();
	}
	
	// Kill the player
	template<CharID Char = DefaultChar<DerivedCompiler>>
	consteval auto KillPlayer()
	{
		return PlayerInstruction<Char, 13>();
	}
};

template<std::size_t scriptSize = 0>
class VanillaScriptCompiler : public BaseScriptCompiler<VanillaScriptCompiler, scriptSize> {};

struct DefaultCompilerTag {};

template<DefaultCompilerTag>
struct DefaultScriptCompiler
{
	using Type = VanillaScriptCompiler<>;
};

template<template<DefaultCompilerTag> class T = DefaultScriptCompiler>
consteval T<{}>::Type NewScript()
{
	return {};
}

} // namespace KuppaScriptImpl

using KuppaScriptImpl::NewScript;

template<const auto scriptCompiler>
inline bool Run()
{
	static constinit auto script = scriptCompiler.End();

	return script.Run();
}

using Mario = KuppaScriptImpl::CharID_Type<0>;
using Luigi = KuppaScriptImpl::CharID_Type<1>;
using Wario = KuppaScriptImpl::CharID_Type<2>;
using Yoshi = KuppaScriptImpl::CharID_Type<3>;

#endif // SM64DS_CUTSCENE_INCLUDED
