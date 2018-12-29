#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <SDL2/SDL_endian.h>

#include <type_traits>

namespace endian {
	struct AnyOrdering {};
	struct NativeOrdering : AnyOrdering {};
	struct ReversedOrdering : AnyOrdering {};

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
	typedef NativeOrdering LittleEndian;
	typedef ReversedOrdering BigEndian;
#else
	typedef NativeOrdering BigEndian;
	typedef ReversedOrdering LittleEndian;
#endif

	typedef BigEndian NetOrdering;

	//template<typename Ordering, typename Type>
	//inline Type reverse( Type v ) { return v; }

	//template<typename Type>
	//typedef WrappedType<ReversedOrdering,Type> ForeignType<Type>;
	//template<typename Type>
	//typedef Type NativeType<Type>;

	/*template<typename T>
	inline T reverse<ReversedOrdering>( T v )
	{
		//return static_cast<Type>(SDL_Swap16( static_cast<Uint16>(v) );
		return 0;
	}

	template<>
	inline float reverse<ReversedOrdering, float>( float v )
	{
		return SDL_SwapFloat( v );
	}*/

	/*template<typename Type>
	inline std::enable_if<sizeof( Type ) == 32, Type>::type reverse<ReversedOrdering, Type>( Type v )
	{
		return static_cast<Type>(SDL_Swap32( static_cast<Uint32>(v) ));
	}

	template<typename Type>
	inline std::enable_if<sizeof( Type ) == 64, Type>::type reverse<ReversedOrdering, Type>( Type v )
	{
		return static_cast<Type>(SDL_Swap64( static_cast<Uint32>(v) ));
	}*/

	template<typename Ordering, typename Type>
	struct rev {
		inline static Type reverse( Type v ) { return v; }
	};

	template<typename Type>
	struct rev<ReversedOrdering, Type> {
		inline static Type reverse( std::enable_if_t<sizeof( Type ) == 16, Type> v )
		{
			return static_cast<Type>(SDL_Swap16( static_cast<Uint16>(v) ));
		}
		inline static Type reverse( std::enable_if_t<sizeof( Type ) == 32, Type> v )
		{
			return static_cast<Type>(SDL_Swap16( static_cast<Uint32>(v) ));
		}
		inline static Type reverse( std::enable_if_t<sizeof( Type ) == 64, Type> v )
		{
			return static_cast<Type>(SDL_Swap16( static_cast<Uint64>(v) ));
		}
	};

	template<>
	struct rev<ReversedOrdering, float> {
		inline static float reverse( float v )
		{
			return SDL_SwapFloat( v );
		}
	};

	template<typename Ordering, typename Type>
	struct WrappedType {
		WrappedType() = default;
		WrappedType( Type v ) : _store( r::reverse( v ) ) {}
		operator Type() const { return r::reverse( _store ); }
		Type get() const { return r::reverse( _store ); }
		Type operator=( Type v ) { _store = r::reverse( v ); return v; }
		Type raw() const { return _store; }

	private:
		typedef rev<Ordering, Type> r;
		Type _store;
	};

}

typedef endian::WrappedType<endian::LittleEndian, uint16_t> LE_uint16;
typedef endian::WrappedType<endian::LittleEndian, int16_t> LE_int16;
typedef endian::WrappedType<endian::LittleEndian, uint32_t> LE_uint32;
typedef endian::WrappedType<endian::LittleEndian, int32_t> LE_int32;
typedef endian::WrappedType<endian::LittleEndian, uint64_t> LE_uint64;
typedef endian::WrappedType<endian::LittleEndian, int64_t> LE_int64;
typedef endian::WrappedType<endian::LittleEndian, float> LE_float;

typedef endian::WrappedType<endian::BigEndian, uint16_t> BE_uint16;
typedef endian::WrappedType<endian::BigEndian, int16_t> BE_int16;
typedef endian::WrappedType<endian::BigEndian, uint32_t> BE_uint32;
typedef endian::WrappedType<endian::BigEndian, int32_t> BE_int32;
typedef endian::WrappedType<endian::BigEndian, uint64_t> BE_uint64;
typedef endian::WrappedType<endian::BigEndian, int64_t> BE_int64;
typedef endian::WrappedType<endian::BigEndian, float> BE_float;

#endif // _ENDIAN_H_
