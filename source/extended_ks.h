#ifndef EXTENDED_KS_INCLUDED
#define EXTENDED_KS_INCLUDED

#include <cstring>
#include "Cutscene.h"
#include "SM64DS_PI.h"

using Any = KuppaScriptImpl::CharID_Type<0xff>;

namespace KuppaScriptImpl {

template<std::size_t scriptSize = 0, class... Initializers>
class ExtendedScriptCompiler : public BaseScriptCompiler<ExtendedScriptCompiler, scriptSize, Initializers...>
{
	using Base = BaseScriptCompiler<ExtendedScriptCompiler, scriptSize, Initializers...>;

public:
	template<uint8_t subID, class... NewInitializers>
	consteval auto CamInstruction(const auto&... args)
	{
		return static_cast<Base&>(*this).template CamInstruction<subID, NewInitializers...>(args...);
	}

	template<CharID Char, uint8_t subID, class... NewInitializers>
	consteval auto PlayerInstruction(const auto&... args)
	{
		return static_cast<Base&>(*this).template PlayerInstruction<Char, subID, NewInitializers...>(args...);
	}

	template<class F, CharID Char = Any>
	consteval auto Call(F)
	{
		static constexpr bool noParams      = std::is_invocable_v<F>;
		static constexpr bool paramIsCam    = std::is_invocable_v<F, Camera&>;
		static constexpr bool paramIsPlayer = std::is_invocable_v<F, Player&>;

		static_assert(paramIsCam + paramIsPlayer + noParams == 1);
		static_assert(!paramIsPlayer || std::same_as<Char, Any>);

		static constexpr auto funcPtr = +[](ActorBase& actor) [[gnu::flatten]]
		{
			if constexpr (noParams)      F{}();
			if constexpr (paramIsCam)    F{}(static_cast<Camera&>(actor));
			if constexpr (paramIsPlayer) F{}(static_cast<Player&>(actor));
		};

		using Initializer = decltype([](char* scriptStart)
		{
			char* addr = scriptStart + scriptSize + 7;

			std::memcpy(addr, &funcPtr, sizeof(funcPtr));
		});

		if constexpr (paramIsPlayer)
			return PlayerInstruction<Char, 14u, Initializer>(0);
		else
			return CamInstruction<39u, Initializer>(0);
	}

	/* -------- -------- Custom player instructions -------- -------- */

	template<CharID Char = Any>
	consteval auto SetPlayerPos(Vector3_16 pos)
	{
		return PlayerInstruction<Char, 15>(pos);
	}

	template<CharID Char = Any>
	consteval auto SetPlayerPos(short x, short y, short z)
	{
		return PlayerInstruction<Char, 15>(x, y, z);
	}

	template<CharID Char = Any>
	consteval auto MovePlayer(Vector3_16 offset)
	{
		return PlayerInstruction<Char, 16>(offset);
	}

	template<CharID Char = Any>
	consteval auto MovePlayer(short x, short y, short z)
	{
		return PlayerInstruction<Char, 16>(x, y, z);
	}

	template<CharID Char = Any>
	consteval auto SetPlayerAngleY(short angleY)
	{
		return PlayerInstruction<Char, 17>(angleY);
	}

	template<CharID Char = Any>
	consteval auto TurnPlayer(short angleOffsetY)
	{
		return PlayerInstruction<Char, 18>(angleOffsetY);
	}

	template<CharID Char = Any>
	consteval auto ExpDecayPlayerAngleY(short targetAngle, uint16_t invFactor, uint16_t maxDelta = 180_deg, uint16_t minDelta = 0)
	{
		return PlayerInstruction<Char, 19>(targetAngle, invFactor, maxDelta, minDelta);
	}

	template<CharID Char = Any>
	consteval auto PlayLong(unsigned soundArchiveID, unsigned soundID)
	{
		return PlayerInstruction<Char, 20>(soundArchiveID, soundID);
	}

	template<CharID Char = Any>
	consteval auto HurtPlayer(Vector3_16 source, unsigned damage = 0, Fix12i speed = 12._f, unsigned arg4 = 1, unsigned presetHurt = 0, unsigned spawnOuchParticles = 1)
	{
		return PlayerInstruction<Char, 21>(source, damage, speed, arg4, presetHurt, spawnOuchParticles);
	}

	template<CharID Char = Any>
	consteval auto BurnPlayer()
	{
		return PlayerInstruction<Char, 22>();
	}

	template<CharID Char = Any>
	consteval auto ShockPlayer(unsigned damage = 0)
	{
		return PlayerInstruction<Char, 23>(damage);
	}

	template<CharID Char = Any>
	consteval auto BouncePlayer(Fix12i initVel)
	{
		return PlayerInstruction<Char, 24>(initVel);
	}

	template<CharID Char = Any>
	consteval auto PrintPlayerPos()
	{
		return PlayerInstruction<Char, 25>();
	}

	consteval auto SetEntranceMode(int8_t entranceMode)
	{
		return PlayerInstruction<Any, 26>(entranceMode);
	}

	template<CharID Char = Any>
	consteval auto DeactivatePlayer()
	{
		return PlayerInstruction<Char, 27>();
	}

	/* -------- -------- Custom camera instructions -------- -------- */

	consteval auto LerpCamPos(Vector3_16 dest, uint8_t smoothness)
	{
		return CamInstruction<40>(dest, smoothness);
	}

	consteval auto LerpCamPos(short x, short y, short z, uint8_t smoothness)
	{
		return CamInstruction<40>(x, y, z, smoothness);
	}

	consteval auto LerpCamTarget(Vector3_16 dest, uint8_t smoothness)
	{
		return CamInstruction<41>(dest, smoothness);
	}

	consteval auto LerpCamTarget(short x, short y, short z, uint8_t smoothness)
	{
		return CamInstruction<41>(x, y, z, smoothness);
	}

	consteval auto DisableAmbientSoundEffects()
	{
		return CamInstruction<42>();
	}

	consteval auto PrintCamPos()
	{
		return CamInstruction<43>();
	}

	consteval auto PrintCamTarget()
	{
		return CamInstruction<44>();
	}

	consteval auto PrintFrameCounter()
	{
		return CamInstruction<45>();
	}

	template<unsigned length>
	consteval auto Print(const char (&string)[length])
	{
		return CamInstruction<46>(std::to_array<const char, length>(string));
	}

	consteval auto SetCamAngleZ(short zAngle)
	{
		return CamInstruction<47>(zAngle);
	}

	consteval auto RotateCamZ(short zAngleDiff)
	{
		return CamInstruction<48>(zAngleDiff);
	}

	consteval auto ExpDecayCamAngleZ(short targetAngle, uint16_t invFactor, uint16_t maxDelta = 180_deg, uint16_t minDelta = 0)
	{
		return CamInstruction<49>(targetAngle, invFactor, maxDelta, minDelta);
	}

	consteval auto SetCamShakeIntensity(short zAngleDiff)
	{
		return CamInstruction<50>(zAngleDiff);
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