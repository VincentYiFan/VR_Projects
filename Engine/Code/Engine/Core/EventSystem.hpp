#pragma once
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include <string>
#include <vector>

typedef unsigned int EntityID;
typedef void(*EventCallbackFunctionPtrType)( NamedProperties& args );


struct EventSubscription
{
	EventSubscription( std::string eventName, EventCallbackFunctionPtrType eventCallbackPtrType, const std::string& inputValue );
	std::string m_eventName; // e.g, "Sunrise"
	NamedProperties m_input;
	EventCallbackFunctionPtrType m_callbackFuncPtr = nullptr;
};

/*static NamedStrings name##_input = NamedStrings::PopulateFromEvent( inputA ); \*/

#define COMMAND( name, inputA ) \
	static void name##_impl( NamedProperties& args ); \
	static EventSubscription name##_register( #name, name##_impl, inputA ); \
	static void name##_impl( NamedProperties& args )

class EventSystem
{
public:
	EventSystem();
	~EventSystem();

	void StartUp();
	void SubscribeToEvent( const std::string& eventName, EventCallbackFunctionPtrType eventCallbackPtrType );
	void FireEvent( const std::string& eventName );
	void FireEvent( const std::string& eventName, NamedProperties value );

	void FireEventWithValue( const std::string& eventNameWithValue );

	std::vector< EventSubscription* > m_eventSubsrciptions;
};
