#pragma once
#include "Engine/Core/StringUtils.hpp"
#include <string>
#include <map>

//------------------------------------------------------------------------
//------------------------------------------------------------------------
class TypedPropertyBase
{
public:
	virtual ~TypedPropertyBase() {}

	virtual std::string GetAsString() const = 0;
	virtual void const* GetUniqueID() const = 0;

	template <typename T>
	bool Is() const;
};

//------------------------------------------------------------------------
//------------------------------------------------------------------------
template <typename VALUE_TYPE>
class TypedProperty : public TypedPropertyBase
{
public:
	virtual std::string GetAsString() const final { return ToString( m_value ).c_str(); }
	virtual void const* GetUniqueID() const final { return StaticUniqueID(); }

public:
	// std::string m_key;
	VALUE_TYPE m_value;

public:
	static void const* const StaticUniqueID()
	{
		static int s_local = 0;
		return &s_local;
	}
};


//------------------------------------------------------------------------
//------------------------------------------------------------------------
class NamedProperties
{
public:
	//------------------------------------------------------------------------
	~NamedProperties()
	{
		for( auto iter : m_keyValuePairs ) {
			delete iter.second;
		}

		m_keyValuePairs.clear();
	}
	//------------------------------------------------------------------------
	void ResetValues()
	{
		for( auto iter : m_keyValuePairs )
		{
			SetValue( iter.first, "" );
		}
	}


	//------------------------------------------------------------------------

	 void PopulateFromEvent( const std::string& commandInputWithValue );

	//------------------------------------------------------------------------
	// for everything else, there's templates!
	template <typename T>
	void SetValue( std::string const& keyName, T const& value )
	{
		TypedPropertyBase* base = FindInMap( keyName );
		if( base == nullptr ) {
			// doesn't exist, make a new one            
			TypedProperty<T>* prop = new TypedProperty<T>();
			prop->m_value = value;
			m_keyValuePairs[keyName] = prop;

		}
		else {
			// it exists?
			if( base->Is<T>() ) {
				TypedProperty<T>* prop = (TypedProperty<T>*)base;
				prop->m_value = value;
			}
			else {
				// not the same thing
				// delete and remake
				delete base;

				// doesn't exist, make a new one            
				TypedProperty<T>* prop = new TypedProperty<T>();
				prop->m_value = value;
				m_keyValuePairs[keyName] = prop;
			}
		} 
	}

	//------------------------------------------------------------------------
	template <typename T>
	T GetValue( std::string const& keyName, T const& defValue ) const
	{
		TypedPropertyBase* base = FindInMap( keyName );
		if( nullptr != base ) {

			// this works WITHOUT RTTI enabled
			// but will not work if prop is inherited from T
			if( base->Is<T>() ) {
				// make sure this is safe!  how....?
				TypedProperty<T>* prop = (TypedProperty<T>*)base;
				return prop->m_value;
			}
			else {
				std::string strValue = base->GetAsString();
				return GetValueFromString( strValue.c_str(), defValue );
			}
		}
		else { // failed to find
			return defValue;
		}
	}

	//------------------------------------------------------------------------
	// specialized for char const
	void SetValue( std::string const& keyname, char const* val )
	{
		SetValue<std::string>( keyname, val );
	}

	//------------------------------------------------------------------------
	std::string GetValue( std::string const& keyName, char const* val ) const
	{
 		return GetValue<std::string>( keyName, val );
	}

private:
	//------------------------------------------------------------------------
	TypedPropertyBase* FindInMap( std::string const& key ) const
	{
		auto iter = m_keyValuePairs.find( key );
		if( iter != m_keyValuePairs.end() ) {
			return iter->second;
		}
		else {
			return nullptr;
		}
	}
	//------------------------------------------------------------------------
	
	// this is going to be different
	// std::map<std::string, std::string> m_keyValuePairs;
	// std::map<std::string, void*> m_keyValuePointers;

	// We need to type type in a map
	// But we can't store the temlate argument... or can we?
	std::map<std::string, TypedPropertyBase*> m_keyValuePairs;
};

template<typename T>
inline bool TypedPropertyBase::Is() const
{
	//TypedProperty<T> prop;
	//return( prop.GetUniqueID() == /*TypedProperty<T>::*/GetUniqueID() );
	return GetUniqueID() == TypedProperty<T>::StaticUniqueID();
}
