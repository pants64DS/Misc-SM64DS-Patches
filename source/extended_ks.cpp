#include "extended_ks.h"
#include "SM64DS_PI.h"

namespace KuppaScriptImpl
{
	using ESC = ExtendedScriptCompiler<>;

	template<class... Args>
	using PendingInstruction = ESC::PendingInstruction<(sizeof(Args) + ... + 1)>;

	template<class... Args>
	using Overload = PendingInstruction<Args...>(ESC::*)(Args...);

	template<uint8_t id>
	struct ObjByID {};

	template<uint8_t id> requires (id < 4 || id == 0xff)
	struct ObjByID<id> { using Type = Player; };

	template<> struct ObjByID<4> { using Type = Camera; };

	template<class Obj, std::size_t numVFuncs>
	consteval std::size_t GetNumVFuncs(KS_MemberFuncPtr<Obj>(&)[numVFuncs])
	{
		return numVFuncs;
	}

	template<class> constexpr std::size_t numVFuncs;

	template<> constexpr std::size_t numVFuncs<Player> = GetNumVFuncs(KS_PLAYER_FUNCTIONS);
	template<> constexpr std::size_t numVFuncs<Camera> = GetNumVFuncs(KS_CAMERA_FUNCTIONS);

	template<class Obj, uint8_t subID>
	struct ID
	{
		using ObjType = Obj;

		static_assert(subID >= numVFuncs<Obj>, "custom instruction sub-ID is already used by a vanilla instruction");
	};

	template<auto ptmf>
	struct GetID_Impl
	{
		static constexpr auto ids = []<class... Args>(Overload<Args...> f)
		{
			const PendingInstruction<Args...> instruction = (ESC().*f)(Args{}...);

			return std::to_array<uint8_t>({instruction.id, instruction.params[0]});
		}
		(ptmf);

		using Type = ID<typename ObjByID<ids[0]>::Type, ids[1]>;
	};

	template<auto ptmf>
	using GetID = GetID_Impl<ptmf>::Type;

	struct UnimplementedTag {};
}

template<class ID>
auto Implementation(typename ID::ObjType&, const char* params, short minFrame, short maxFrame)
{
	return KuppaScriptImpl::UnimplementedTag{};
}

#define IMPLEMENT(functionName) template<> auto \
Implementation<KuppaScriptImpl::GetID<&KuppaScriptImpl::ESC::functionName>>

#define IMPLEMENT_ID(Obj, subID) template<> auto \
Implementation<KuppaScriptImpl::ID<Obj, subID>>

#define IMPLEMENT_OVERLOAD(functionName, ...) template<> auto \
Implementation<KuppaScriptImpl::GetID<KuppaScriptImpl::Overload<__VA_ARGS__>{&KuppaScriptImpl::ESC::functionName}>>

#include "extended_ks.impl"

namespace KuppaScriptImpl
{
	template<class Obj>
	using CFunc = void(Obj&, const char* params, short minFrame, short maxFrame);

	template<class R, class... Args>
	consteval bool IsImplemented(R(&)(Args...))
	{
		return !std::same_as<R, UnimplementedTag>;
	}

	template<class Obj>
	void Nop(Obj&, const char* params, short minFrame, short maxFrame) {}

	template<class Obj, unsigned firstSubID = numVFuncs<Obj>, std::size_t nops = 0>
	constexpr auto customInstructionTable = [] consteval
	{
		if constexpr (firstSubID > 0xff)
			return std::array<CFunc<Obj>*, 0>{};

		else if constexpr (IsImplemented(Implementation<ID<Obj, firstSubID>>))
		{
			std::array<CFunc<Obj>*, nops + 1> res;

			std::fill_n(res.begin(), nops, &Nop<Obj>);
			res.back() = &Implementation<ID<Obj, firstSubID>>;

			return res + customInstructionTable<Obj, firstSubID + 1, 0>;
		}
		else
			return customInstructionTable<Obj, firstSubID + 1, nops + 1>;
	}();

	template<class Obj>
	static void CallInstruction(Obj& obj, char* instruction, short minFrame, short maxFrame)
	{
		const unsigned vFuncID = instruction[6];

		if (vFuncID < numVFuncs<Obj>)
		{
			obj.CallKuppaScriptInstruction(instruction, minFrame, maxFrame);

			return;
		}

		const unsigned cFuncID = vFuncID - numVFuncs<Obj>;

		const auto& cFuncs = customInstructionTable<Obj>;

		if (cFuncID < cFuncs.size())
			cFuncs[cFuncID](obj, instruction + 7, minFrame, maxFrame);
	}
}

using KuppaScriptImpl::CallInstruction;

int repl_0200e5f0(Player& player, char* instruction, short minFrame, short maxFrame)
{
	CallInstruction(player, instruction, minFrame, maxFrame);

	return 1; // just in case 0200e5ac is called from an unknown location
}

void repl_0200ed4c(Camera& cam, char* instruction, short minFrame, short maxFrame)
{
	CallInstruction(cam, instruction, minFrame, maxFrame);
}

asm(R"(
@ Check if the camera exists before updating the script from RunKuppaScript
repl_0200ef5c:
	ldr   r0, =CAMERA
	ldr   r0, [r0]
	cmp   r0, #0
	bne   0x0200e8d4
	b     0x0200ef60

@ Support player instructions that don't refer to a specific character
nsub_0200ed70:
	cmp   r0, #0xff
	bne   0x0200ed78
	ldr   r0, =PLAYER_ARR
	mov   r1, r4
	ldr   r0, [r0]
	mov   r2, r9
	cmp   r0, #0
	mov   r3, r10
	blne  _Z13repl_0200e5f0R6PlayerPcss
	b     0x0200ed88

@ Undo ActivatePlayer for player 0 every time a cutscene ends
nsub_0200e7b0:
	ldr   r0, =PLAYER_ARR
	ldr   r0, [r0]
	cmp   r0, #0
	ldrne r2, [r0, #0xb0]
	bicne r2, r2, #0x24000000
	strne r2, [r0, #0xb0]
	ldr   r0, =#0x020890a0
	b     0x0200e7b4
)");
