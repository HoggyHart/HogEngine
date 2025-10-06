#pragma once
#include <string>
#include <map>

template<typename ObjectType>
class ObjectManager {
private:
	std::string appendNumForUniqueID(std::string ID, std::size_t copyNum) {
		if (!getObject(ID + "_" + copyNum)) return appendNumForUniqueID(ID, ++copyNum);
		else return ID + "_" + copyNum;
	}
public:
	ObjectManager() {};
	std::map < std::string, ObjectType*> objectRegistry{};

	

	inline std::string addObject(std::string ID, ObjectType* obj) {
		if (getObject(ID)!=nullptr) {
			ID = appendNumForUniqueID(ID, 1);
		}
		objectRegistry[ID] = obj;
		return ID;
	}
	inline ObjectType* getObject(std::string ID) const { return objectRegistry[ID]; }
};