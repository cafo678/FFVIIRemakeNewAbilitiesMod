#include "Types.h"
#include <string>

// Classes

template<typename T>
struct TArray {
	T* Data;
	uint32 Num;
	uint32 Max;
};

struct FString : TArray<wchar_t> {};

struct FNameEntryHandle {
	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset) {};
	FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535) {};
	operator uint32_t() const { return (Block << 16 | Offset); }
};

struct FNameEntry 
{
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
	union
	{
		char AnsiName[1024];
		wchar_t	WideName[1024];
	};

	std::string String();
};

struct FNamePool
{
	char Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	char* Blocks[8192];

	FNameEntry* GetEntry(FNameEntryHandle handle) const;
};

struct FName {
	uint32 Index;
	uint32 Number;

	bool operator != (const FName& other) const { return bool(Index != other.Index || Number != other.Number); }
	bool operator == (const FName& other) const { return bool(Index == other.Index && Number == other.Number); }

	std::string GetName();
};

struct UObject {
	void** VFTable;
	uint32 ObjectFlags;
	uint32 InternalIndex;
	struct UClass* ClassPrivate;
	FName NamePrivate;
	UObject* OuterPrivate;

	std::string GetName();
	std::string GetFullName();
	bool IsA(void* cmp);
};

struct FUObjectItem
{
	UObject* Object;
	__int32 Flags;
	__int32 ClusterIndex;
	__int32 SerialNumber;
	char unknowndata_00[0x4];
};

struct TUObjectArray 
{
	FUObjectItem* Objects;
	uint32 MaxElements;
	uint32 NumElements;

	UObject* GetObjectPtr(uint32_t id) const;
	UObject* FindObject(const char* name) const;
};

// Size: 0x30 (Inherited: 0x28)
struct UField : UObject 
{
	char UnknownData_28[0x8]; // 0x28(0x08)
};

// Size: 0x90 (Inherited: 0x30)
struct UStruct : UField 
{
	char UnknownData_30[0x10];	// 0x30(0x10)
	UStruct* SuperStruct;		// 0x40(0x8)
	char UnknownData_48[0x48];	// 0x48(0x48)
};

struct UFunction : UStruct
{
	uint32_t FunctionFlags;
	uint16_t NumParms;
	uint16_t ParmsSize;
	char UnknownData00[0x20];
	void* Func;
};

// Size: 0x230 (Inherited: 0x90)
struct UClass : UStruct 
{
	char UnknownData_B0[0x170]; // 0x90(0x170)
};

struct FFrame
{
	char pad_0x0000[0x10];
	UFunction* Node;
	UObject* Object;
	uint8_t* Code;
	uint8_t* Locals;
};

struct FVector 
{
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	FVector(float InF) : X(InF), Y(InF), Z(InF) { }
	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }
	float DistTo(const FVector& V) const { return (*this - V).Size(); }
	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }
};

// Globals

extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;