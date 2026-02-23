#include "Engine_.h"

// TUObjectArray

UObject* TUObjectArray::FindObject(const char* name) const
{
	for (auto i = 0u; i < NumElements; i++)
	{
		auto object = GetObjectPtr(i);

		if (object && object->GetFullName() == name) 
		{
			return object;
		}
	}
	return nullptr;
}

UObject* TUObjectArray::GetObjectPtr(uint32_t id) const
{
	if (id >= NumElements) 
		return nullptr;

	return Objects[id].Object;
}

// UObject

std::string UObject::GetFullName()
{
	std::string name;
	for (auto outer = OuterPrivate; outer; outer = outer->OuterPrivate) 
	{ 
		name = outer->GetName() + "." + name; 
	}

	name = ClassPrivate->GetName() + " " + name + this->GetName();
	return name;
}

bool UObject::IsA(void* cmp)
{
	for (auto super = ClassPrivate; super; super = static_cast<UClass*>(super->SuperStruct)) 
	{ 
		if (super == cmp) 
			return true;
	}
	return false;
}

std::string UObject::GetName()
{
	return NamePrivate.GetName();
}

// FNameEntry

std::string FNameEntry::String()
{
	if (bIsWide) { return std::string(); }
	return { AnsiName, Len };
}

FNameEntry* FNamePool::GetEntry(FNameEntryHandle handle) const
{
	return reinterpret_cast<FNameEntry*>(Blocks[handle.Block] + 2 * static_cast<uint64_t>(handle.Offset));
}

// FName

std::string FName::GetName()
{
	auto entry = NamePoolData->GetEntry(Index);
	auto name = entry->String();
	if (Number > 0)
	{
		name += '_' + std::to_string(Number);
	}
	auto pos = name.rfind('/');
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1);
	}
	return name;
}

// Globals

TUObjectArray* ObjObjects = nullptr;
FNamePool* NamePoolData = nullptr;