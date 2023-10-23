#ifndef EXTENDED_KS_INCLUDED
#define EXTENDED_KS_INCLUDED

#include "include/Cutscene.h"

using Any = KuppaScriptImpl::CharID_Type<0xff>;

namespace KuppaScriptImpl {

template<std::size_t scriptSize = 0>
class ExtendedScriptCompiler : public BaseScriptCompiler<ExtendedScriptCompiler, scriptSize>
{
	using Base = BaseScriptCompiler<ExtendedScriptCompiler, scriptSize>;

public:
	template<uint8_t subID, class... Args>
	consteval auto CamInstruction(const Args&... args)
	{
		return static_cast<Base&>(*this).template CamInstruction<subID, Args...>(args...);
	}

	template<CharID Char, uint8_t subID, class... Args>
	consteval auto PlayerInstruction(const Args&... args)
	{
		return static_cast<Base&>(*this).template PlayerInstruction<Char, subID, Args...>(args...);
	}

	/* -------- -------- Custom player instructions -------- -------- */

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

	template<CharID Char = Any>
	consteval auto MovePlayer(Vector3_16 offset)
	{
		return PlayerInstruction<Char, 15>(offset);
	}

	template<CharID Char = Any>
	consteval auto MovePlayer(short x, short y, short z)
	{
		return PlayerInstruction<Char, 15>(x, y, z);
	}

	template<CharID Char = Any>
	consteval auto SetPlayerAngleY(short angleY)
	{
		return PlayerInstruction<Char, 16>(angleY);
	}

	template<CharID Char = Any>
	consteval auto TurnPlayer(short angleOffsetY)
	{
		return PlayerInstruction<Char, 17>(angleOffsetY);
	}

	template<CharID Char = Any>
	consteval auto ExpDecayPlayerAngleY(short targetAngle, int invFactor, int maxDelta = 180_deg, int minDelta = 0)
	{
		return PlayerInstruction<Char, 18>(targetAngle, invFactor, maxDelta, minDelta);
	}

	template<CharID Char = Any>
	consteval auto PlayLong(unsigned soundArchiveID, unsigned soundID)
	{
		return PlayerInstruction<Char, 19>(soundArchiveID, soundID);
	}

	template<CharID Char = Any>
	consteval auto HurtPlayer(Vector3_16 source, unsigned damage = 0, Fix12i speed = 12._f, unsigned arg4 = 1, unsigned presetHurt = 0, unsigned spawnOuchParticles = 1)
	{
		return PlayerInstruction<Char, 20>(source, damage, speed, arg4, presetHurt, spawnOuchParticles);
	}

	template<CharID Char = Any>
	consteval auto BurnPlayer()
	{
		return PlayerInstruction<Char, 21>();
	}

	template<CharID Char = Any>
	consteval auto ShockPlayer(unsigned damage = 0)
	{
		return PlayerInstruction<Char, 22>(damage);
	}

	template<CharID Char = Any>
	consteval auto BouncePlayer(Fix12i initVel)
	{
		return PlayerInstruction<Char, 23>(initVel);
	}

	template<CharID Char = Any>
	consteval auto PrintPlayerPos()
	{
		return PlayerInstruction<Char, 24>();
	}

	/* -------- -------- Custom camera instructions -------- -------- */

	consteval auto LerpCamPos(Vector3_16 dest, uint8_t smoothness)
	{
		return CamInstruction<39>(dest, smoothness);
	}

	consteval auto LerpCamPos(short x, short y, short z, uint8_t smoothness)
	{
		return CamInstruction<39>(x, y, z, smoothness);
	}

	consteval auto LerpCamTarget(Vector3_16 dest, uint8_t smoothness)
	{
		return CamInstruction<40>(dest, smoothness);
	}

	consteval auto LerpCamTarget(short x, short y, short z, uint8_t smoothness)
	{
		return CamInstruction<40>(x, y, z, smoothness);
	}

	consteval auto DisableAmbientSoundEffects()
	{
		return CamInstruction<41>();
	}

	consteval auto PrintCamPos()
	{
		return CamInstruction<42>();
	}

	consteval auto PrintCamTarget()
	{
		return CamInstruction<43>();
	}

	consteval auto PrintFrameCounter()
	{
		return CamInstruction<44>();
	}

	template<unsigned length>
	consteval auto Print(const char (&string)[length])
	{
		return CamInstruction<45>(std::to_array<const char, length>(string));
	}
};

template<> struct DefaultScriptCompiler<{}>
{
	using Type = ExtendedScriptCompiler<>;
};

template<> struct DefaultCharImpl<ExtendedScriptCompiler>
{
	using Type = Any;
};

} // namespace KuppaScriptImpl

#endif