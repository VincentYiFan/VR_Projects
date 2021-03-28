#pragma once
#include <iostream>

struct LocaleBool {
	bool data;
	LocaleBool() {}
	LocaleBool( bool data ): data( data ) {}
	operator bool() const { return data; }
	friend std::ostream& operator << ( std::ostream& out, LocaleBool b ) {
		out << std::boolalpha << b.data;
		return out;
	}
	friend std::istream& operator >> ( std::istream& in, LocaleBool& b ) {
		in >> std::boolalpha >> b.data;
		return in;
	}
};
