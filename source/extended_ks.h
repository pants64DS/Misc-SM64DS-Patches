#ifndef EXTENDED_KS_INCLUDED
#define EXTENDED_KS_INCLUDED

#include "include/Cutscene.h"

namespace KuppaScriptImpl {

template<std::size_t scriptSize = 0>
class ExtendedScriptCompiler : public BaseScriptCompiler<ExtendedScriptCompiler, scriptSize>
{
	using Base = BaseScriptCompiler<ExtendedScriptCompiler, scriptSize>;

public:
	template<uint8_t subID, class... Args>
	consteval auto CamInstruction(const Args&... args) const
	{
		return static_cast<const Base&>(*this).template CamInstruction<subID, Args...>(args...);
	}

	template<CharacterID character, uint8_t subID, class... Args>
	consteval auto PlayerInstruction(const Args&... args) const
	{
		return static_cast<const Base&>(*this).template PlayerInstruction<character, subID, Args...>(args...);
	}

	/* -------- -------- Custom player instructions -------- -------- */

	template<CharacterID character>
	consteval auto SetPlayerPos(Vector3_16 pos) const
	{
		return PlayerInstruction<character, 14>(pos);
	}

	template<CharacterID character>
	consteval auto SetPlayerPos(short x, short y, short z) const
	{
		return PlayerInstruction<character, 14>(x, y, z);
	}

	template<CharacterID character>
	consteval auto MovePlayer(Vector3_16 offset) const
	{
		return PlayerInstruction<character, 15>(offset);
	}

	template<CharacterID character>
	consteval auto MovePlayer(short x, short y, short z) const
	{
		return PlayerInstruction<character, 15>(x, y, z);
	}

	template<CharacterID character>
	consteval auto SetPlayerAngleY(short angleY) const
	{
		return PlayerInstruction<character, 16>(angleY);
	}

	template<CharacterID character>
	consteval auto TurnPlayer(short angleOffsetY) const
	{
		return PlayerInstruction<character, 17>(angleOffsetY);
	}

	template<CharacterID character>
	consteval auto ExpDecayPlayerAngleY(short targetAngle, int invFactor, int maxDelta = 180_deg, int minDelta = 0) const
	{
		return PlayerInstruction<character, 18>(targetAngle, invFactor, maxDelta, minDelta);
	}

	template<CharacterID character>
	consteval auto PlayLong(unsigned soundArchiveID, unsigned soundID) const
	{
		return PlayerInstruction<character, 19>(soundArchiveID, soundID);
	}

	template<CharacterID character>
	consteval auto HurtPlayer(Vector3_16 source, unsigned damage = 0, Fix12i speed = 12._f, unsigned arg4 = 1, unsigned presetHurt = 0, unsigned spawnOuchParticles = 1) const
	{
		return PlayerInstruction<character, 20>(source, damage, speed, arg4, presetHurt, spawnOuchParticles);
	}

	template<CharacterID character>
	consteval auto BurnPlayer() const
	{
		return PlayerInstruction<character, 21>();
	}

	template<CharacterID character>
	consteval auto ShockPlayer(unsigned damage = 0) const
	{
		return PlayerInstruction<character, 22>(damage);
	}

	template<CharacterID character>
	consteval auto BouncePlayer(Fix12i initVel) const
	{
		return PlayerInstruction<character, 23>(initVel);
	}

	template<CharacterID character>
	consteval auto PrintPlayerPos() const
	{
		return PlayerInstruction<character, 24>();
	}

	/* -------- -------- Custom camera instructions -------- -------- */

	consteval auto LerpCamPos(Vector3_16 dest, uint8_t smoothness) const
	{
		return CamInstruction<39>(dest, smoothness);
	}

	consteval auto LerpCamPos(short x, short y, short z, uint8_t smoothness) const
	{
		return CamInstruction<39>(x, y, z, smoothness);
	}

	consteval auto LerpCamTarget(Vector3_16 dest, uint8_t smoothness) const
	{
		return CamInstruction<40>(dest, smoothness);
	}

	consteval auto LerpCamTarget(short x, short y, short z, uint8_t smoothness) const
	{
		return CamInstruction<40>(x, y, z, smoothness);
	}

	consteval auto DisableAmbientSoundEffects() const
	{
		return CamInstruction<41>();
	}

	consteval auto PrintCamPos() const
	{
		return CamInstruction<42>();
	}

	consteval auto PrintCamTarget() const
	{
		return CamInstruction<43>();
	}

	consteval auto PrintFrameCounter() const
	{
		return CamInstruction<44>();
	}

	template<unsigned length>
	consteval auto Print(const char (&string)[length]) const
	{
		return CamInstruction<45>(std::to_array<const char, length>(string));
	}
};

template<> struct DefaultScriptCompiler<{}>
{
	using Type = ExtendedScriptCompiler<>;
};

} // namespace KuppaScriptImpl

#endif