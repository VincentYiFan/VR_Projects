#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"

static uint constexpr MAX_REGISTERED_EVENTS( 128 );
static EventSubscription* gRegistrarList[MAX_REGISTERED_EVENTS];
static uint gRegistrarCount = 0;

EventSystem* g_theEventSystem = nullptr;

EventSubscription::EventSubscription( std::string eventName, EventCallbackFunctionPtrType eventCallbackPtrType, const std::string& inputValue )
	:m_callbackFuncPtr( eventCallbackPtrType )
	, m_eventName(eventName)
{
	m_input.PopulateFromEvent( inputValue );

	gRegistrarList[gRegistrarCount] = this;
	gRegistrarCount++;
}


EventSystem::EventSystem()
{
	for( uint idx = 0; idx < gRegistrarCount; ++idx )
	{
		m_eventSubsrciptions.push_back( gRegistrarList[idx] );
	}
}

EventSystem::~EventSystem()
{
}

void EventSystem::StartUp()
{

}

void EventSystem::SubscribeToEvent( const std::string& eventName, EventCallbackFunctionPtrType eventCallbackPtrType )
{
	EventSubscription* newSubscrition = new EventSubscription( eventName, eventCallbackPtrType, nullptr );
	newSubscrition->m_eventName = eventName;
	newSubscrition->m_callbackFuncPtr = eventCallbackPtrType;
	m_eventSubsrciptions.push_back( newSubscrition );
}


void EventSystem::FireEvent( const std::string& eventName )
{
	for( int i = 0; i < (int)m_eventSubsrciptions.size(); i++ )
	{
		EventSubscription* subscription = m_eventSubsrciptions[ i ];
		if( subscription->m_eventName == eventName )
		{
			m_eventSubsrciptions[i]->m_callbackFuncPtr( m_eventSubsrciptions[i]->m_input );
		}
	}
}


void EventSystem::FireEvent( const std::string& eventName, NamedProperties value )
{
	for( int i = 0; i < (int)m_eventSubsrciptions.size(); i++ )
	{
		EventSubscription* subscription = m_eventSubsrciptions[i];
		if( subscription->m_eventName == eventName )
		{
			m_eventSubsrciptions[i]->m_callbackFuncPtr( value );
		}
	}
}

void EventSystem::FireEventWithValue( const std::string& eventNameWithValue )
{
	Strings strings = SplitStringOnDelimiter( eventNameWithValue, ' ' );

	EventSubscription* subscription = nullptr;
	std::string eventName = strings[0];

	//std::string rawEventValue = eventNameWithValue.substr( eventName.size() + 1, eventNameWithValue.size() - 1 );

	for( int i = 0; i < (int)m_eventSubsrciptions.size(); ++i )
	{
		if( m_eventSubsrciptions[i]->m_eventName == eventName )
		{
			subscription = m_eventSubsrciptions[i];
		}
	}

	if( subscription != nullptr ) 
	{
		subscription->m_input.ResetValues();
		NamedProperties& inputValue = subscription->m_input;

		//Strings eventValue = SplitStringOnDelimiter( rawEventValue, ',' );
		//for( int i = 0; i < (int)eventValue.size(); ++i ) {
		//	Strings commandValue = SplitStringOnDelimiter( eventValue[i], '=' );
		//	if( commandValue.size() > 1 ) {
		//		inputValue.SetValue( commandValue[0], commandValue[1] );
		//	}
		//}

		for( int i = 1; i < (int)strings.size(); ++i ) {
			Strings commandValue = SplitStringOnDelimiter( strings[i], '=' );
			if( commandValue.size() > 1 ) {
				inputValue.SetValue( commandValue[0], commandValue[1] );
			}
		}

		subscription->m_callbackFuncPtr( subscription->m_input );
	}
}
