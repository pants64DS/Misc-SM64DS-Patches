#ifndef SM64DS_COMMON_INCLUDED
#define SM64DS_COMMON_INCLUDED

#include "ostream.h"
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <utility>
#include <limits>

// Note: vector and matrix structures defined in this file use
// attributes on lambda expressions as in proposal P2173R0

class Actor;
class Player;

namespace cstd
{
	int div(int numerator, int denominator);
	int mod(int numerator, int denominator);
	int fdiv(int numerator, int denominator);     // returns a Q12 number
	int64_t ldiv(int numerator, int denominator); // returns a Q32 number
}

extern "C"
{
	uint16_t DecIfAbove0_Short(uint16_t& counter); // returns the counter's new value
	uint8_t DecIfAbove0_Byte(uint8_t& counter);    // returns the counter's new value
}

bool ApproachLinear(short& counter, short dest, short step); // returns whether the counter reached its destination
bool ApproachLinear(int& counter,   int dest,   int step);   // returns whether the counter reached its destination

struct AsRaw {} constexpr as_raw;

template<class T> // a valid underlying representation of a fixed-point number
concept FixUR = std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= sizeof(int);

template<FixUR T, int q, template<FixUR> class CRTP>
struct Fix
{
	T val;

	using Promoted = CRTP<int>;

	constexpr Fix() = default;
	constexpr Fix(T val) : val(val << q) {}
	constexpr Fix(T val, AsRaw) : val(val) {}
	constexpr explicit Fix(long double val) : val(val * (1ll << q) + 0.5l) {}

	template<FixUR U, int r, template<class> class CRTP2>
	constexpr Fix(Fix<U, r, CRTP2> f, AsRaw) : val(f.val) {}

	template<FixUR U>
	constexpr Fix(CRTP<U> f) : val(f.val) {}

	friend constexpr
	Promoted operator+ (CRTP<T> f) { return {f.val, as_raw}; }

	friend constexpr
	Promoted operator- (CRTP<T> f) { return {-f.val, as_raw}; }

	template<FixUR U> friend constexpr
	Promoted operator+ (CRTP<T> f0, CRTP<U> f1) { return {f0.val + f1.val, as_raw}; }

	template<FixUR U> friend constexpr
	Promoted operator- (CRTP<T> f0, CRTP<U> f1) { return {f0.val - f1.val, as_raw}; }

	template<FixUR U> friend constexpr
	CRTP<T>& operator+=(CRTP<T>& f0, CRTP<U> f1) { f0.val += f1.val; return f0; }
 
	template<FixUR U> friend constexpr
	CRTP<T>& operator-=(CRTP<T>& f0, CRTP<U> f1) { f0.val -= f1.val; return f0; }

	friend constexpr Promoted operator+ (int i,  CRTP<T> f) { return Promoted(i) + f; }
	friend constexpr Promoted operator- (int i,  CRTP<T> f) { return Promoted(i) - f; }
	friend constexpr Promoted operator+ (CRTP<T>  f, int i) { return f + Promoted(i); }
	friend constexpr Promoted operator- (CRTP<T>  f, int i) { return f - Promoted(i); }
	friend constexpr CRTP<T>& operator+=(CRTP<T>& f, int i) { return f += Promoted(i); }
	friend constexpr CRTP<T>& operator-=(CRTP<T>& f, int i) { return f -= Promoted(i); }

	friend constexpr Promoted operator* (int i,  CRTP<T> f) { return {i * f.val, as_raw}; }
	friend constexpr Promoted operator* (CRTP<T>  f, int i) { return {f.val * i, as_raw}; }
	friend constexpr Promoted operator/ (CRTP<T>  f, int i) { return {f.val / i, as_raw}; }
	friend constexpr CRTP<T>& operator*=(CRTP<T>& f, int i) { f.val *= i; return f; }
	friend constexpr CRTP<T>& operator/=(CRTP<T>& f, int i) { f.val /= i; return f; }

	friend constexpr Promoted operator<< (CRTP<T>  f, int i) { return {f.val << i, as_raw}; }
	friend constexpr Promoted operator>> (CRTP<T>  f, int i) { return {f.val >> i, as_raw}; }
	friend constexpr CRTP<T>& operator<<=(CRTP<T>& f, int i) { f.val <<= i; return f; }
	friend constexpr CRTP<T>& operator>>=(CRTP<T>& f, int i) { f.val >>= i; return f; }

	template<FixUR U> [[gnu::always_inline, nodiscard]] friend
	Promoted operator*(CRTP<T> f0, CRTP<U> f1)
	{
		const uint64_t product = static_cast<int64_t>(f0.val) * f1.val;
		Promoted result;

		asm(R"(
			movs %[lo], %[lo], lsr %[s0]
			adc  %[rs], %[lo], %[hi], lsl %[s1]
		)":
		[rs] "=r" (result) :
		[lo] "r" (static_cast<uint32_t>(product)),
		[hi] "r" (static_cast<uint32_t>(product >> 32)),
		[s0] "I" (q),
		[s1] "I" (32 - q) : "cc");

		return result;
	}

	template<FixUR U> friend inline
	CRTP<T>& operator*=(CRTP<T>& f0, CRTP<U> f1) { return f0 = f0 * f1; }

	template<FixUR U> friend constexpr
	bool operator==(CRTP<T> f0, CRTP<U> f1) { return f0.val == f1.val; }

	template<FixUR U> friend constexpr
	bool operator< (CRTP<T> f0, CRTP<U> f1) { return f0.val <  f1.val; }

	template<FixUR U> friend constexpr
	bool operator<=(CRTP<T> f0, CRTP<U> f1) { return f0.val <= f1.val; }

	template<FixUR U> friend constexpr
	bool operator> (CRTP<T> f0, CRTP<U> f1) { return f0.val >  f1.val; }

	template<FixUR U> friend constexpr
	bool operator>=(CRTP<T> f0, CRTP<U> f1) { return f0.val >= f1.val; }


	static constexpr CRTP<T> max {std::numeric_limits<T>::max(), as_raw};
	static constexpr CRTP<T> min {std::numeric_limits<T>::min(), as_raw};

	constexpr Promoted friend Abs(CRTP<T> f) { return f.val >= 0 ? f : -f; }
	constexpr explicit operator T() const { return val >> q; }
	constexpr explicit operator bool() const { return val != 0; }

	bool ApproachLinear(Promoted dest, Promoted step) & { return ::ApproachLinear(val, dest.val, step.val); }
};

template<FixUR T>
struct Fix12 : Fix<T, 12, Fix12>
{
	using Fix<T, 12, Fix12>::Fix;

	Fix12<int> operator/ (Fix12 f) const { return {cstd::fdiv(this->val, f.val), as_raw}; }
	Fix12&     operator/=(Fix12 f) & { this->val = cstd::fdiv(this->val, f.val); return *this; }
};

using Fix12i = Fix12<int>;
using Fix12s = Fix12<short>;

consteval Fix12i operator""_f (unsigned long long val) { return Fix12i(val, as_raw); }
consteval Fix12s operator""_fs(unsigned long long val) { return Fix12s(val, as_raw); }

consteval Fix12i operator""_f (long double val) { return Fix12i(val); }
consteval Fix12s operator""_fs(long double val) { return Fix12s(val); }

consteval int operator""_deg(long double val) { return val * 32768.L / 180.L; }
consteval int operator""_deg(unsigned long long val) { return operator""_deg(static_cast<long double>(val)); }

consteval int operator""_rad(long double val) { return val * 32768.L / 3.141592653589793238462643383279502884L; }
consteval int operator""_rad(unsigned long long val) { return operator""_rad(static_cast<long double>(val)); }

namespace cstd
{
	Fix12i fdiv(Fix12i numerator, Fix12i denominator);
	unsigned sqrt(uint64_t x); // 64 bit unsigned sqrt
	inline Fix12i sqrt(Fix12i x) { return Fix12i(sqrt(static_cast<uint64_t>(x.val) << 12), as_raw); }

	short atan2(Fix12i y, Fix12i x); //atan2 function, what about 0x020538b8?
	int abs(int x);

	int strcmp(const char* str1, const char* str2); //returns 0 if equal, a positive number if str1 comes after str2, and a negative number otherwise
	char* strncpy(char* dest, const char* src, unsigned count);	//Copies n bytes from src to dest and returns a pointer to dest
	char* strchr(const char* str, char c); //Searches for c in str and returns a pointer to the first occurence, or 0 if c could not be found
	unsigned strlen(const char* str); //Returns the length of the string or -1 if no null-terminator has been found
}

struct UnknownStruct
{
	unsigned unk00;
	unsigned unk04;
	uint16_t unk08;
	uint16_t buttonsHeld;
	uint16_t cameraAngleY;
	uint16_t unk0e;
	unsigned unk10;
	unsigned unk14;
	unsigned unk18;
	unsigned unk1c;
	unsigned unk20;
};

struct Vector3;
struct Vector3_16;
struct Matrix4x3;
struct Matrix3x3;
struct SharedFilePtr;

extern "C"
{	
	extern uint16_t POWERS_OF_TEN[3]; //100, 10, 1
	extern char DIGIT_ENC_ARR[10];

	extern uint16_t HEALTH_ARR[4];
	extern int UNUSED_RAM[0xec00];
	extern UnknownStruct UNKNOWN_ARR[4];
	
	extern int RNG_STATE; //x => x * 0x0019660d + 0x3c6ef35f
	
	extern Matrix4x3 MATRIX_SCRATCH_PAPER;
	extern unsigned* HEAP_PTR;
	extern unsigned FRAME_COUNTER;

	extern const Fix12s SINE_TABLE[0x2000];
	extern const Fix12s ATAN_TABLE[0x400];
	
	void UnloadObjBankOverlay(int ovID);
	bool LoadObjBankOverlay(int ovID);
	char* LoadFile	(int ov0FileID);
	
	[[noreturn]] void Crash();

	void FreeFileAllocation(void* ptr);
	void FreeHeapAllocation(void* ptr, unsigned* heapPtr);
	void* AllocateFileSpace(unsigned amount);
	
	short AngleDiff(short ang0, short ang1) __attribute__((const));
	void Vec3_RotateYAndTranslate(Vector3& res, const Vector3& translation, short angY, const Vector3& v); //res and v cannot alias.
	short Vec3_VertAngle(const Vector3& v1, const Vector3& v0) __attribute__((pure));
	short Vec3_HorzAngle(const Vector3& v0, const Vector3& v1) __attribute__((pure));
	int RandomIntInternal(int* randomIntStatePtr);
	void Matrix4x3_FromTranslation(Matrix4x3& mF, Fix12i x, Fix12i y, Fix12i z);
	void Matrix4x3_FromRotationZ(Matrix4x3& mF, short angZ);
	void Matrix4x3_FromRotationY(Matrix4x3& mF, short angY);
	void Matrix4x3_FromRotationX(Matrix4x3& mF, short angX);
	void Matrix4x3_FromRotationZXYExt(Matrix4x3& mF, short angX, short angY, short angZ); //yxz intrinsic = zxy extrinsic
	void Matrix4x3_FromRotationXYZExt(Matrix4x3& mF, short angX, short angY, short angZ); //zyx intrinsic = xyz extrinsic
	void Matrix4x3_ApplyInPlaceToRotationZ(Matrix4x3& mF, short angZ); //transforms a rotation matrix using matrix mF.
	void Matrix4x3_ApplyInPlaceToRotationY(Matrix4x3& mF, short angY); //does not apply a rotation matrix.
	void Matrix4x3_ApplyInPlaceToRotationX(Matrix4x3& mF, short angX); //don't get the two confused.
	
	Fix12i Vec3_HorzDist(const Vector3& v0, const Vector3& v1) __attribute__((pure));
	Fix12i Vec3_HorzLen(const Vector3& v0) __attribute__((pure));
	Fix12i Vec3_Dist(const Vector3& v0, const Vector3& v1) __attribute__((pure));
	bool Vec3_Equal(const Vector3& v0, const Vector3& v1) __attribute__((pure));
	void Vec3_LslInPlace(Vector3& v, int shift);
	void Vec3_Lsl(Vector3& res, const Vector3& v, int shift);
	void Vec3_AsrInPlace(Vector3& v, int shift);
	void Vec3_Asr(Vector3& res, const Vector3& v, int shift);
	void Vec3_DivScalarInPlace(Vector3& v, Fix12i scalar);
	void Vec3_MulScalarInPlace(Vector3& v, Fix12i scalar);
	void Vec3_MulScalar(Vector3& res, const Vector3& v, Fix12i scalar);
	void Vec3_Sub(Vector3& res, const Vector3& v0, const Vector3& v1); // not as efficient as AddVec3
	void Vec3_Add(Vector3& res, const Vector3& v0, const Vector3& v1); // not as efficient as SubVec3
	
	void Matrix3x3_LoadIdentity(Matrix3x3& mF);
	void MulVec3Mat3x3(const Vector3& v, const Matrix3x3& m, Vector3& res);
	void MulMat3x3Mat3x3(const Matrix3x3& m1, const Matrix3x3& m0, Matrix3x3& mF); //m0 is applied to m1, so it's m0*m1=mF
	void Matrix4x3_LoadIdentity(Matrix4x3& mF);
	 // long call to force gcc to actually call the off-by-one address and therefore set the mode to thumb.
	void Matrix4x3_FromScale(Matrix4x3& mF, Fix12i x, Fix12i y, Fix12i z) __attribute__((long_call, target("thumb")));
	void MulVec3Mat4x3(const Vector3& v, const Matrix4x3& m, Vector3& res);
	void MulMat4x3Mat4x3(const Matrix4x3& m1, const Matrix4x3& m0, Matrix4x3& mF); //m0 is applied to m1, so it's m0*m1=mF
	void InvMat4x3(const Matrix4x3& m0, Matrix4x3& mF);		//Loads inverse of m0 into mF
	void NormalizeVec3(const Vector3& v, Vector3& res);
	void CrossVec3(const Vector3& v0, const Vector3& v1, Vector3& res);
	void AddVec3(const Vector3& v0, const Vector3& v1, Vector3& res);
	void SubVec3(const Vector3& v0, const Vector3& v1, Vector3& res);
	Fix12i LenVec3(const Vector3& v);
	Fix12i DotVec3(const Vector3& v0, const Vector3& v1) __attribute__((pure));
	
	void Matrix3x3_SetRotationX(Matrix3x3& m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb"))); //Resets m to an X rotation matrix
	void Matrix3x3_SetRotationY(Matrix3x3& m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb"))); //Resets m to a Y rotation matrix
	void Matrix3x3_SetRotationZ(Matrix3x3& m, Fix12i sinTheta, Fix12i cosTheta) __attribute__((long_call, target("thumb"))); //Resets m to a Z rotation matrix
	
	void MultiStore_Int(int val, void* dest, int byteSize);
	void MultiCopy_Int(void* source, void* dest, int byteSize);
	
	uint16_t Color_Interp(uint16_t* dummyArg, uint16_t startColor, uint16_t endColor, Fix12i time) __attribute__((const));
}

inline int RandomInt() { return RandomIntInternal(&RNG_STATE); }

struct Vector2     { Fix12i x, y; };
struct Vector2_16  { short  x, y; };
struct Vector3_16  { short  x, y, z; };
struct Vector3_16f { Fix12s x, y, z; };

template<class T>
struct UnaliasedRef
{
	T& r;

	constexpr UnaliasedRef(T& r) [[gnu::always_inline]] : r(r) {}

	[[gnu::always_inline]]
	T& operator=(auto&& proxy)
	{
		proxy.template Eval<false>(r);

		return r;
	}
};

template<class T>
concept HasProxy = []() consteval
{
	using F = decltype([]<bool>(auto&){});

	if constexpr (requires { typename T::Proxy<F>; })
		return std::constructible_from<T, typename T::Proxy<F>>;
	else
		return false;
}();

template<HasProxy T> [[gnu::always_inline, nodiscard]]
constexpr UnaliasedRef<T> AssureUnaliased(T& t) { return t; }

template<class T> [[gnu::always_inline, nodiscard]]
constexpr T& AssureUnaliased(T& t) { return t; }

[[nodiscard]]
constexpr int Lerp(int a, int b, Fix12i t)
{
	return static_cast<int>(t * (b - a)) + a;
}

struct Vector3
{
	template<class F>
	class Proxy
	{
		F eval;

		template<class G>
		using NewProxy = Proxy<G>;

	public:
		[[gnu::always_inline]]
		constexpr explicit Proxy(F&& eval) : eval(eval) {}

		template<bool resMayAlias> [[gnu::always_inline]]
		void Eval(Vector3& res) { eval.template operator()<resMayAlias>(res); }

		[[gnu::always_inline, nodiscard]]
		Fix12i Dist(const Vector3& v) && { return static_cast<Vector3>(std::move(*this)).Dist(v); }

		[[gnu::always_inline, nodiscard]]
		Fix12i HorzDist(const Vector3& v) && { return static_cast<Vector3>(std::move(*this)).HorzDist(v); }

		[[gnu::always_inline, nodiscard]]
		Fix12i Len() && { return static_cast<Vector3>(std::move(*this)).Len(); }

		[[gnu::always_inline, nodiscard]]
		Fix12i HorzLen() && { return static_cast<Vector3>(std::move(*this)).HorzLen(); }

		[[gnu::always_inline, nodiscard]]
		Fix12i Dot (const Vector3& v) && { return static_cast<Vector3>(std::move(*this)).Dot(v); }

		[[gnu::always_inline, nodiscard]]
		short  HorzAngle(const Vector3& v) && { return static_cast<Vector3>(std::move(*this)).HorzAngle(v); }

		[[gnu::always_inline, nodiscard]]
		short  VertAngle(const Vector3& v) && { return static_cast<Vector3>(std::move(*this)).VertAngle(v); }

		[[gnu::always_inline, nodiscard]]
		auto Cross(const Vector3& v) &&
		{
			return NewProxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				if constexpr (resMayAlias)
				{
					const Vector3 temp = v;
					Eval<resMayAlias>(res);
					CrossVec3(res, temp, res);
				}
				else
				{
					Eval<resMayAlias>(res);
					CrossVec3(res, v, res);	
				}
			});
		}

		template<class G>
		[[gnu::always_inline, nodiscard]]
		auto Cross(Proxy<G>&& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				CrossVec3(res, std::move(other), res);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto Normalized() &&
		{
			return NewProxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res.Normalize();
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto NormalizedTwice() &&
		{
			return NewProxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res.NormalizeTwice();
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto AngleTo(const Vector3& other) &&
		{
			return static_cast<Vector3>(std::move(*this)).AngleTo(other);
		}

		[[gnu::always_inline, nodiscard]]
		auto operator-() &&
		{
			return NewProxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res.x = -res.x;
				res.y = -res.y;
				res.z = -res.z;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator+(const Vector3& v) && { return v + std::move(*this); }

		[[gnu::always_inline, nodiscard]]
		auto operator-(const Vector3& v) &&
		{
			return NewProxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				if constexpr (resMayAlias)
				{
					const Vector3 temp = v;
					Eval<resMayAlias>(res);
					res -= temp;
				}
				else
				{
					Eval<resMayAlias>(res);
					res -= v;
				}
			});
		}

		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator+(Proxy<G>&& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res += static_cast<Vector3>(std::move(other));
			});
		}

		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator-(Proxy<G>&& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res -= static_cast<Vector3>(std::move(other));
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator*(const Fix12i& scalar) &&
		{
			return NewProxy([this, &scalar]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res *= scalar;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator/(const Fix12i& scalar) &&
		{
			return NewProxy([this, &scalar]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res /= scalar;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator<<(const int& shift) &&
		{
			return NewProxy([this, &shift]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res <<= shift;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator>>(const int& shift) &&
		{
			return NewProxy([this, &shift]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				Eval<resMayAlias>(res);
				res >>= shift;
			});
		}
	};

	Fix12i x, y, z;

	constexpr Vector3() = default;
	constexpr Vector3(auto x, auto y, auto z) : x(x), y(y), z(z) {}

	[[gnu::noinline, gnu::noclone]]
	constexpr Vector3(const Vector3_16& v): x(v.x), y(v.y), z(v.z) {}

	template<class F> [[gnu::always_inline]]
	Vector3(Proxy<F>&& proxy) { proxy.template Eval<false>(*this); }

	template<class F> [[gnu::always_inline]]
	Vector3& operator=(Proxy<F>&& proxy) & { proxy.template Eval<true>(*this); return *this; }

	[[gnu::always_inline, nodiscard]]
	static constexpr auto Temp(const auto& x, const auto& y, const auto& z)
	{
		return Proxy([&x, &y, &z]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			res.x = x;
			res.y = y;
			res.z = z;
		});
	}

	[[gnu::always_inline, nodiscard]]
	static constexpr auto Raw(const auto& x, const auto& y, const auto& z)
	{
		return Proxy([&x, &y, &z]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			res.x.val = x.val;
			res.y.val = y.val;
			res.z.val = z.val;
		});
	}

	[[gnu::always_inline, nodiscard]]
	static constexpr auto Raw(const auto& v)
	{
		return Proxy([&v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			const auto& [x, y, z] = v;

			res.x.val = x.val;
			res.y.val = y.val;
			res.z.val = z.val;
		});
	}

	Vector3& operator+= (const Vector3& v) &     { AddVec3(*this, v, *this); return *this; }
	Vector3& operator-= (const Vector3& v) &     { SubVec3(*this, v, *this); return *this; }
	Vector3& operator*= (Fix12i scalar)    &     { Vec3_MulScalarInPlace(*this, scalar); return *this; }
	Vector3& operator/= (Fix12i scalar)    &     { Vec3_DivScalarInPlace(*this, scalar); return *this; }
	Vector3& operator<<=(int shift)        &     { Vec3_LslInPlace(*this, shift); return *this; }
	Vector3& operator>>=(int shift)        &     { Vec3_AsrInPlace(*this, shift); return *this; }
	Fix12i   Dist       (const Vector3& v) const { return Vec3_Dist(*this, v); }
	Fix12i   HorzDist   (const Vector3& v) const { return Vec3_HorzDist(*this, v); }
	Fix12i   Len        ()                 const { return LenVec3(*this); }
	Fix12i   HorzLen    ()                 const { return Vec3_HorzLen(*this); }
	Fix12i   Dot        (const Vector3& v) const { return DotVec3(*this, v); }
	short    HorzAngle  (const Vector3& v) const { return Vec3_HorzAngle(*this, v); }
	short    VertAngle  (const Vector3& v) const { return Vec3_VertAngle(*this, v); }

	Vector3& operator*=(const auto& m) & { return *this = m * *this; }

	[[gnu::always_inline]]
	bool operator== (const Vector3& other) const& { return Vec3_Equal(*this, other); }

	// use an inlinable version if either operand is a proxy
	template<class T, class F> [[gnu::always_inline]] friend
	bool operator== (T&& any, Proxy<F>&& proxy)
	{
		const Vector3& v0 = std::forward<T>(any);
		const Vector3& v1 = std::move(proxy);

		return v0.x == v1.x && v0.y == v1.y && v0.z == v1.z;
	}

	[[gnu::always_inline, nodiscard]]
	auto operator-() const
	{
		return Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			res.x = -x;
			res.y = -y;
			res.z = -z;
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator+(const Vector3& v) const
	{
		return Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			AddVec3(*this, v, res);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator-(const Vector3& v) const
	{
		return Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			SubVec3(*this, v, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator+(Proxy<F>&& proxy) const
	{
		return Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				res += temp;
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				res += *this;
			}
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator-(Proxy<F>&& proxy) const
	{
		return Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				SubVec3(temp, res, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				SubVec3(*this, res, res);
			}
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator* (const Fix12i& scalar) const
	{
		return Proxy([this, &scalar]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			Vec3_MulScalar(res, *this, scalar);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator/ (const Fix12i& scalar) const
	{
		return Proxy([this, &scalar]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			res = *this; // no DivScalar exists.
			res /= scalar;
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator<<(const int& shift) const
	{
		return Proxy([this, &shift]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			Vec3_Lsl(res, *this, shift);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator>>(const int& shift) const
	{
		return Proxy([this, &shift]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			Vec3_Asr(res, *this, shift);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto Normalized() const
	{
		return Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			NormalizeVec3(*this, res);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto NormalizedTwice() const
	{
		return Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			NormalizeVec3(*this, res);
			res.Normalize();
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto Cross(const Vector3& v) const
	{
		return Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			CrossVec3(*this, v, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto Cross(Proxy<F>&& proxy)
	{
		return Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				CrossVec3(temp, res, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				CrossVec3(*this, res, res);	
			}
		});
	}

	[[gnu::always_inline]]
	void Normalize() & { NormalizeVec3(*this, *this); }
	void NormalizeTwice() & { Normalize(); Normalize(); }

	[[gnu::always_inline, nodiscard]]
	auto RotateYAndTranslate(const Vector3& translation, const short& angY) const
	{
		return Proxy([this, &translation, &angY]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			Vec3_RotateYAndTranslate(res, translation, angY, *this);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto RotateYAndTranslate(Proxy<F>&& proxy, const short& angY) const
	{
		return Proxy([this, &proxy, &angY]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				Vec3_RotateYAndTranslate(res, res, angY, temp);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				Vec3_RotateYAndTranslate(res, res, angY, *this);
			}
		});
	}

	int AngleTo(const Vector3& other) const
	{
		return cstd::atan2(this->Cross(other).Len(), this->Dot(other)) & 0xffff;
	}

	[[gnu::always_inline, nodiscard]]
	friend auto Lerp(const Vector3& a, const Vector3& b, const auto& t)
	{
		return Proxy([&a, &b, &t]<bool resMayAlias> [[gnu::always_inline]] (auto& res)
		{
			if constexpr (resMayAlias)
				res = t * (b - a) + a;
			else
				AssureUnaliased(res) = t * (b - a) + a;
		});
	}

	template<class Rotator> requires(sizeof(Rotator) > 4) [[gnu::noinline, gnu::noclone]]
	Vector3& RotateAround(const Vector3& pivot, const Rotator& rotator)
	{
		return *this = Lerp(pivot, *this, rotator);
	}

	template<class Rotator> requires(sizeof(Rotator) <= 4) [[gnu::noinline, gnu::noclone]]
	Vector3& RotateAround(const Vector3& pivot, Rotator rotator)
	{
		return *this = Lerp(pivot, *this, rotator);
	}
};

[[gnu::always_inline, nodiscard]]
inline auto operator* (const Fix12i& scalar, const Vector3& v) { return v * scalar; }

template<class F> [[gnu::always_inline, nodiscard]]
inline auto operator* (const Fix12i& scalar, Vector3::Proxy<F>&& proxy) { return std::move(proxy) * scalar; }

struct Matrix2x2 // Matrix is column-major!
{
	Vector2 c0;
	Vector2 c1;
};

struct Matrix3x3 // Matrix is column-major!
{
	Vector3 c0, c1, c2;

	template<class F>
	class Proxy
	{
		F eval;

		template<class G>
		using NewProxy = Proxy<G>;

	public:

		[[gnu::always_inline]]
		constexpr explicit Proxy(F&& eval) : eval(eval) {}

		template<bool resMayAlias> [[gnu::always_inline]]
		void Eval(Matrix3x3& res) { eval.template operator()<resMayAlias>(res); }

		[[gnu::always_inline, nodiscard]]
		auto operator()(const Matrix3x3& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
			{
				if constexpr (resMayAlias)
				{
					const Matrix3x3 temp = other;
					Eval<resMayAlias>(res);
					MulMat3x3Mat3x3(temp, res, res);
				}
				else
				{
					Eval<resMayAlias>(res);
					MulMat3x3Mat3x3(other, res, res);
				}
			});
		}

		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator()(Proxy<G>&& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
			{
				const Matrix3x3 temp = std::move(other);
				Eval<resMayAlias>(res);
				MulMat3x3Mat3x3(temp, res, res);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator()(const Vector3& v) &&
		{
			return Vector3::Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				if constexpr (resMayAlias)
				{
					const Vector3 temp = v; // v may be altered while the second argument is calculated
					MulVec3Mat3x3(temp, std::move(*this), res);
				}
				else
					MulVec3Mat3x3(v, std::move(*this), res);
			});
		}
		
		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator()(Vector3::Proxy<G>&& proxy) &&
		{
			return Vector3::Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				const Matrix3x3 matrix = std::move(*this);
				proxy.template Eval<resMayAlias>(res);
				MulVec3Mat3x3(res, matrix, res);
			});
		}

		template<class T> [[gnu::always_inline, nodiscard]]
		auto operator*(T&& x) &&
		{
			return std::move(*this)(std::forward<T>(x));
		}
	};

	constexpr Matrix3x3() = default;
	constexpr Matrix3x3(const Vector3& c0, const Vector3& c1, const Vector3& c2) : c0(c0), c1(c1), c2(c2) {}

	constexpr Matrix3x3 (
		Fix12i c0x, Fix12i c1x, Fix12i c2x,
		Fix12i c0y, Fix12i c1y, Fix12i c2y,
		Fix12i c0z, Fix12i c1z, Fix12i c2z
	):
		c0(c0x, c0y, c0z),
		c1(c1x, c1y, c1z),
		c2(c2x, c2y, c2z)
	{}

	template<class F> [[gnu::always_inline]]
	Matrix3x3(Proxy<F>&& proxy) { proxy.template Eval<false>(*this); }

	template<class F> [[gnu::always_inline]]
	Matrix3x3& operator=(Proxy<F>&& proxy) & { proxy.template Eval<true>(*this); return *this; }
	
	static const Matrix3x3 IDENTITY;

	[[gnu::always_inline, nodiscard]]
	static auto Identity()
	{
		return Proxy([]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
		{
			Matrix3x3_LoadIdentity(res);
		});
	}

	template<class C0, class C1, class C2> [[gnu::always_inline, nodiscard]]
	static auto Temp(C0&& c0, C1&& c1, C2&& c2)
	{
		return Proxy([&]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
		{
			if constexpr (resMayAlias)
			{
				res.c0 = std::forward<C0>(c0);
				res.c1 = std::forward<C1>(c1);
				res.c2 = std::forward<C2>(c2);
			}
			else
			{
				AssureUnaliased(res.c0) = std::forward<C0>(c0);
				AssureUnaliased(res.c1) = std::forward<C1>(c1);
				AssureUnaliased(res.c2) = std::forward<C2>(c2);
			}
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator()(const Matrix3x3& other) const
	{
		return Proxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
		{
			MulMat3x3Mat3x3(other, *this, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator()(Proxy<F>& proxy) const
	{
		return Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Matrix3x3& res)
		{
			if constexpr (resMayAlias)
			{
				const Matrix3x3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				MulMat3x3Mat3x3(res, temp, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				MulMat3x3Mat3x3(res, *this, res);	
			}
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator()(const Vector3& v) const
	{
		return Vector3::Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			MulVec3Mat3x3(v, *this, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator()(Vector3::Proxy<F>&& proxy) const
	{
		return Vector3::Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = std::move(proxy);
				MulVec3Mat3x3(temp, *this, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				MulVec3Mat3x3(res, *this, res);
			}
		});
	}

	template<class T> [[gnu::always_inline, nodiscard]]
	auto operator*(T&& x) const
	{
		return operator()(std::forward<T>(x));
	}
	
	class TransposeProxy
	{
		const Matrix3x3& original;

	public:
		constexpr TransposeProxy(const Matrix3x3& original) : original(original) {}

		[[gnu::noinline, gnu::noclone]]
		friend Vector3& operator*=(Vector3& v, TransposeProxy t)
		{
			return v = Vector3::Temp(v.Dot(t.original.c0), v.Dot(t.original.c1), v.Dot(t.original.c2));
		}

		[[gnu::always_inline, nodiscard]]
		auto operator()(const Vector3& v) const
		{
			return Vector3::Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				res *= *this;
			});
		}

		template<class F> [[gnu::always_inline, nodiscard]]
		auto operator()(Vector3::Proxy<F>&& proxy) const
		{
			return Vector3::Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				proxy.template Eval<resMayAlias>(res);
				res = operator()(res);
			});
		}

		template<class T> [[gnu::always_inline, nodiscard]]
		auto operator*(T&& x) const { return operator()(std::forward<T>(x)); }

		[[gnu::always_inline, nodiscard]]
		auto C0() const
		{
			return Vector3::Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				res.x = original.c0.x;
				res.y = original.c1.x;
				res.z = original.c2.x;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto C1() const
		{
			return Vector3::Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				res.x = original.c0.y;
				res.y = original.c1.y;
				res.z = original.c2.y;
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto C2() const
		{
			return Vector3::Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				res.x = original.c0.z;
				res.y = original.c1.z;
				res.z = original.c2.z;
			});
		}

		const Matrix3x3& GetOriginal() const { return original; }
	};

	TransposeProxy Transpose() const { return *this; }

	void TransposeInPlace() &
	{
		void* trash; // input operands can't be clobbers

		asm volatile (R"(
			ldmib  %0,  {r1-r7}
			stmib  %0!, {r3, r6}
			stmib  %0!, {r1, r4, r7}
			stmib  %0,  {r2, r5}
		)"
		: "=r" (trash) : "0" (this) : "r1", "r2", "r3", "r4", "r5", "r6", "r7");
	}
};


// Actually a 4x4 matrix with (0, 0, 0, 1) as the last row
struct Matrix4x3 : private Matrix3x3 // Matrix is column-major!
{
	using Matrix3x3::c0;
	using Matrix3x3::c1;
	using Matrix3x3::c2;
	Vector3 c3;

	static const Matrix4x3 IDENTITY;

	template<class F>
	class Proxy
	{
		F eval;

		template<class G>
		using NewProxy = Proxy<G>;

	public:
		[[gnu::always_inline]]
		constexpr explicit Proxy(F&& eval) : eval(eval) {}

		template<bool resMayAlias> [[gnu::always_inline]]
		void Eval(Matrix4x3& res) { eval.template operator()<resMayAlias>(res); }

		[[gnu::always_inline, nodiscard]]
		auto Inverse() const
		{
			return NewProxy([this]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				Eval<resMayAlias>(res);
				InvMat4x3(res, res);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto RotateZ(const short& angZ) &&
		{
			return NewProxy([this, &angZ]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				Eval<resMayAlias>(res);
				res.RotateZ(angZ);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto RotateY(const short& angY) &&
		{
			return NewProxy([this, &angY]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				Eval<resMayAlias>(res);
				res.RotateY(angY);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto RotateX(const short& angX) &&
		{
			return NewProxy([this, &angX]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				Eval<resMayAlias>(res);
				res.RotateX(angX);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator()(const Matrix4x3& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				if constexpr (resMayAlias)
				{
					const Matrix4x3 temp = other;
					Eval<resMayAlias>(res);
					MulMat4x3Mat4x3(temp, res, res);
				}
				else
				{
					Eval<resMayAlias>(res);
					MulMat4x3Mat4x3(other, res, res);
				}
			});
		}

		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator()(Proxy<G>&& other) &&
		{
			return NewProxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
			{
				const Matrix4x3 temp = std::move(other);
				Eval<resMayAlias>(res);
				MulMat4x3Mat4x3(temp, res, res);
			});
		}

		[[gnu::always_inline, nodiscard]]
		auto operator()(const Vector3& v) &&
		{
			return Vector3::Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				if constexpr (resMayAlias)
				{
					const Vector3 temp = v; // v may be altered while the second argument is calculated
					MulVec3Mat4x3(temp, std::move(*this), res);
				}
				else
					MulVec3Mat4x3(v, std::move(*this), res);
			});
		}
		
		template<class G> [[gnu::always_inline, nodiscard]]
		auto operator()(Vector3::Proxy<G>&& proxy) &&
		{
			return Vector3::Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
			{
				const Matrix4x3 matrix = std::move(*this);
				proxy.template Eval<resMayAlias>(res);
				MulVec3Mat4x3(res, matrix, res);
			});
		}

		template<class T> [[gnu::always_inline, nodiscard]]
		auto operator*(T&& x) &&
		{
			return std::move(*this)(std::forward<T>(x));
		}
	};

	constexpr Matrix4x3() = default;
	constexpr Matrix4x3(const Vector3& c0, const Vector3& c1, const Vector3& c2, const Vector3& c3):
		Matrix3x3(c0, c1, c2),
		c3(c3)
	{}

	constexpr Matrix4x3 (
		Fix12i c0x, Fix12i c1x, Fix12i c2x, Fix12i c3x,
		Fix12i c0y, Fix12i c1y, Fix12i c2y, Fix12i c3y,
		Fix12i c0z, Fix12i c1z, Fix12i c2z, Fix12i c3z
	):
		Matrix3x3 (
			c0x, c1x, c2x,
			c0y, c1y, c2y,
			c0z, c1z, c2z
		),
		c3(c3x, c3y, c3z)
	{}

	constexpr explicit Matrix4x3(const Matrix3x3& linear):
		Matrix3x3(linear),
		c3{}
	{}

	template<class F>
	constexpr explicit Matrix4x3(Matrix3x3::Proxy<F>&& proxy):
		Matrix3x3(std::move(proxy)),
		c3{}
	{}

	template<class F> [[gnu::always_inline]]
	Matrix4x3(Proxy<F>&& proxy) { proxy.template Eval<false>(*this); }

	template<class F> [[gnu::always_inline]]
	Matrix4x3& operator=(Proxy<F>&& proxy) & { proxy.template Eval<true>(*this); return *this; }

	      Matrix3x3& Linear()       { return *this; }
	const Matrix3x3& Linear() const { return *this; }

	Matrix4x3& RotateZ(short angZ) & { Matrix4x3_ApplyInPlaceToRotationZ(*this, angZ); return *this; }
	Matrix4x3& RotateY(short angY) & { Matrix4x3_ApplyInPlaceToRotationY(*this, angY); return *this; }
	Matrix4x3& RotateX(short angX) & { Matrix4x3_ApplyInPlaceToRotationX(*this, angX); return *this; }
	
	[[gnu::always_inline, nodiscard]]
	auto Inverse() const
	{
		return Proxy([this]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			InvMat4x3(*this, res);
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator()(const Matrix4x3& other) const
	{
		return Proxy([this, &other]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			MulMat4x3Mat4x3(other, *this, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator()(Proxy<F>& proxy) const
	{
		return Proxy([this, &proxy]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			if constexpr (resMayAlias)
			{
				const Matrix4x3 temp = *this;
				proxy.template Eval<resMayAlias>(res);
				MulMat4x3Mat4x3(res, temp, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				MulMat4x3Mat4x3(res, *this, res);
			}
		});
	}

	[[gnu::always_inline, nodiscard]]
	auto operator()(const Vector3& v) const
	{
		return Vector3::Proxy([this, &v]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			MulVec3Mat4x3(v, *this, res);
		});
	}

	template<class F> [[gnu::always_inline, nodiscard]]
	auto operator()(Vector3::Proxy<F>&& proxy) const
	{
		return Vector3::Proxy([&proxy, this]<bool resMayAlias> [[gnu::always_inline]] (Vector3& res)
		{
			if constexpr (resMayAlias)
			{
				const Vector3 temp = std::move(proxy);
				MulVec3Mat4x3(temp, *this, res);
			}
			else
			{
				proxy.template Eval<resMayAlias>(res);
				MulVec3Mat4x3(res, *this, res);
			}
		});
	}

	template<class T> [[gnu::always_inline, nodiscard]]
	auto operator*(T&& x) const
	{
		return operator()(std::forward<T>(x));
	}

	[[gnu::always_inline, nodiscard]]
	static auto Identity()
	{
		return Proxy([]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_LoadIdentity(res);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto Translation(const Fix12i& x, const Fix12i& y, const Fix12i& z)
	{
		return Proxy([&x, &y, &z]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromTranslation(res, x, y, z);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto Translation(const Vector3& translation)
	{
		return Proxy([&translation]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromTranslation(res, translation.x, translation.y, translation.z);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto Scale(const Fix12i& x, const Fix12i& y, const Fix12i& z)
	{
		return Proxy([&x, &y, &z]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromScale(res, x, y, z);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto Scale(const Vector3& scale)
	{
		return Proxy([&scale]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromScale(res, scale.x, scale.y, scale.z);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto RotationX(const short& angX)
	{
		return Proxy([&angX]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromRotationX(res, angX);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto RotationY(const short& angY)
	{
		return Proxy([&angY]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromRotationY(res, angY);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto RotationZ(const short& angZ)
	{
		return Proxy([&angZ]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromRotationZ(res, angZ);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto RotationZXY(const short& angX, const short& angY, const short& angZ)
	{
		return Proxy([&angX, &angY, &angZ]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromRotationZXYExt(res, angX, angY, angZ);
		});
	}

	[[gnu::always_inline, nodiscard]]
	static auto RotationXYZ(const short& angX, const short& angY, const short& angZ)
	{
		return Proxy([&angX, &angY, &angZ]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			Matrix4x3_FromRotationXYZExt(res, angX, angY, angZ);
		});
	}

	template<class C0, class C1, class C2, class C3> [[gnu::always_inline, nodiscard]]
	static auto Temp(C0&& c0, C1&& c1, C2&& c2, C3&& c3)
	{
		return Proxy([&]<bool resMayAlias> [[gnu::always_inline]] (Matrix4x3& res)
		{
			if constexpr (resMayAlias)
			{
				res.c0 = std::forward<C0>(c0);
				res.c1 = std::forward<C1>(c1);
				res.c2 = std::forward<C2>(c2);
				res.c3 = std::forward<C3>(c3);
			}
			else
			{
				AssureUnaliased(res.c0) = std::forward<C0>(c0);
				AssureUnaliased(res.c1) = std::forward<C1>(c1);
				AssureUnaliased(res.c2) = std::forward<C2>(c2);
				AssureUnaliased(res.c3) = std::forward<C3>(c3);	
			}
		});
	}
};

struct SharedFilePtr
{
	uint16_t fileID;
	uint8_t numRefs;
	char* filePtr;
	
	SharedFilePtr& Construct(unsigned ov0FileID);
	char* Load();
	void Release();
};

template<class T, T zero = static_cast<T>(0)>
constexpr int Sgn(T val)
{
    return (val > zero) - (val < zero);
}


inline const Fix12s& Sin(short angle)
{
	return SINE_TABLE[static_cast<uint16_t>(angle + 8) >> 4 << 1];
}

inline const Fix12s& Cos(short angle)
{
	return SINE_TABLE[1 + (static_cast<uint16_t>(angle + 8) >> 4 << 1)];
}

constexpr uint16_t Color5Bit(uint8_t r, uint8_t g, uint8_t b) //0x00 to 0xff each
{
	return (uint16_t)r >> 3 << 0 |
		   (uint16_t)g >> 3 << 5 |
		   (uint16_t)b >> 3 << 10;
}

constexpr uint16_t Arr3_5Bit(uint8_t val0, uint8_t val1, uint8_t val2)
{
	return (uint16_t)val0 << 0 |
		   (uint16_t)val1 << 5 |
		   (uint16_t)val2 << 10;
}

template<FixUR T>
inline const ostream& operator<<(const ostream& os, Fix12<T> fix)
{
	if (fix >= Fix12<T>(0, as_raw))
	{
		os.set_buffer("0x%r0%_f");
		os.flush(fix.val);
	}
	else
	{
		os.set_buffer("-0x%r0%_f");
		os.flush(-fix.val); 
	}

	return os;
}

inline const ostream& operator<<(const ostream& os, const Vector3& vec)
{
	os.set_buffer("{0x%r0%_f, 0x%r1%_f, 0x%r2%_f}");
	os.flush(vec.x.val, vec.y.val, vec.z.val);

	return os;
}

inline const ostream& operator<<(const ostream& os, const Vector3_16& vec)
{
	os.set_buffer("{0x%r0%, 0x%r1%, 0x%r2%}");
	os.flush(vec.x, vec.y, vec.z);

	return os;
}

inline const ostream& operator<<(const ostream& os, const Matrix4x3& m)
{
	os.set_buffer("[ 0x%r0%_f  0x%r1%_f  0x%r2%_f  0x%r3%_f ]\n");

	os.flush(m.c0.x.val, m.c1.x.val, m.c2.x.val, m.c3.x.val);
	os.flush(m.c0.y.val, m.c1.y.val, m.c2.y.val, m.c3.y.val);
	os.flush(m.c0.z.val, m.c1.z.val, m.c2.z.val, m.c3.z.val);

	return os;
}

inline const ostream& operator<<(const ostream& os, const Matrix3x3& m)
{
	os.set_buffer("[ 0x%r0%_f  0x%r1%_f  0x%r2%_f ]\n");

	os.flush(m.c0.x.val, m.c1.x.val, m.c2.x.val);
	os.flush(m.c0.y.val, m.c1.y.val, m.c2.y.val);
	os.flush(m.c0.z.val, m.c1.z.val, m.c2.z.val);

	return os;
}

#endif