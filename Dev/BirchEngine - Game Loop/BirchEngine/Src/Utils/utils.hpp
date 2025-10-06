#pragma once
#include <array>
#include <vector>
#include <map>
#include <iostream>
#include <functional>

constexpr auto STATICNULLPTR = nullptr;

template<typename T, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T, T2>& p) {
	return os << "{" << p.first << "," << p.second << "}";
}
template<typename T, std::size_t T2>
std::ostream& operator<<(std::ostream& os, const std::array<T, T2>& p) {
	if (p.size() == 0) return os << "{}";
	os << "{";
	std::array<T, T2>::const_iterator it = p.begin();
	int i = 0;
	while (++i < p.size()) {
		os << *it << ", ";
		++it;
	}
	os << *it << "}";
	return os;
}

template<typename T>
bool vectorHasVal(const std::vector<T>& v, T val) {
	return (std::find(v.begin(), v.end(), val) != v.end());
}

template<typename T, std::size_t SIZE>
bool arrayHasVal(const std::array<T, SIZE>& arr, const T val) {
	return (std::find(arr.begin(), arr.end(), val) != arr.end());
}

template <class T>
inline std::ostream& operator<< (std::ostream& os, const std::vector<T>& s) {
	os << '{';
	if (s.size() > 0) {
		os << s[0];
		for (int i = 1; i < s.size(); i++) {
			os << ", " << s[i];
		}
	}
	return os << '}';
}

struct Unique_ID_Object {
private:
	unsigned long long id;
public:
	Unique_ID_Object() {
		static unsigned long long id = 0;
		this->id = id++;
	}

	unsigned long long getID() { return id; }
};

template<typename T>
struct NoneVariant : public T{
private:
	inline T operator=(const T& obj) {
		*pointer = obj;
	}

	inline void operator=(T* objPointer) {
		if (pointer) delete pointer;
		pointer = obj;
	}
};