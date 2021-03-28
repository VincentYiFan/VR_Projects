#pragma once
#include <functional>
#include <vector>

//------------------------------------------------------------------------
typedef unsigned int uint;
//------------------------------------------------------------------------
// ...ARGS -> "List Of" Args
// the ...ARGS may be 0 or more arguments of type `typename` - allowing us to use this
// with any number of argument types
template<typename ...ARGS>
class Delegate
{
public:
	using function_t = std::function<void( ARGS... )>;	// expand the ARGS out to be the signature for the function type

														// typedef void (*c_callback_t)(ARGS...)	// this line is equivalent to the above code: using...
	using c_callback_t = void (*)(ARGS...);

	struct sub_t // subscription_t
	{
		void const* obj_id		= nullptr;		// object I'm calling on
		void const* func_id		= nullptr;		// address of the function I'm calling
		function_t callable;	// callable function to call above function/method

		inline bool operator==( sub_t const& other ) const { return (func_id == other.func_id) && (obj_id == other.obj_id); }
	};

public:
	//------------------------------------------------------------------------
	void subscribe( c_callback_t const& cb )
	{
		sub_t sub;
		sub.func_id = cb;
		sub.callable = cb;

		subscribe( sub );
	}

	//------------------------------------------------------------------------
	void unsubscribe( c_callback_t const& cb )
	{
		sub_t sub;
		sub.func_id = cb;

		unsubscribe( sub );
	}

	//------------------------------------------------------------------------
	template<typename OBJ_TYPE>
	void subscribe_method( OBJ_TYPE* obj, void (OBJ_TYPE::* mcb) (ARGS...) )
	{
		sub_t sub;
		sub.obj_id = obj;
		sub.func_id = *(void const**)&mcb;

		sub.callable = [ = ]( ARGS ...args ) { (obj->*mcb)(args...); };

		subscribe( sub );
	}

	//------------------------------------------------------------------------
	template <typename OBJ_TYPE>
	void unsubscribe_method( OBJ_TYPE* obj, void (OBJ_TYPE::* mcb)(ARGS...) )
	{
		sub_t sub;
		sub.obj_id = obj;
		sub.func_id = *(void const**)&mcb;

		unsubscribe( sub );
	}

	//------------------------------------------------------------------------
	void invoke( ARGS const& ...args )	// will call all subscribers
	{
		for( sub_t& sub : m_subscriptions ) {
			sub.callable( args... );
		}
	}

	void operator() ( ARGS const& ...args ) { invoke( args... ); }	// allow us to use this object as a function

private:
	// moving this private as no one will use these directly
	void subscribe( sub_t const& sub )	// sub refers to func_to_call
	{
		m_subscriptions.push_back( sub );
	}

	void unsubscribe( sub_t const& sub )
	{
		for( uint i = 0; i < m_subscriptions.size(); ++i ) {
			if( m_subscriptions[i] == sub ) {
				m_subscriptions.erase( m_subscriptions.begin() + i );
				return;
			}
		}
	}

	std::vector<sub_t> m_subscriptions;
};

