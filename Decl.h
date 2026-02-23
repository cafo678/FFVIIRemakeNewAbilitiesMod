#include "Types.h"
#include "Engine_.h"
#include <stdio.h>
#include <vector>

// Structs Declarations

struct Stats
{
	uint16 Unk_00;
	uint8 ATBSlotsNumber;
	uint8 Unk_01;
	float LimitGauge;
	uint64 Unk_02;
	uint32 InfHP;
	uint32 MaxHP;
	uint32 InfMP;
	uint32 MaxMP;
	uint32 XP;
	float ATB;
	uint32 ATK;
	uint32 MATK;
	uint32 DEF;
	uint32 MDEF;
	uint32 Luck;
};

// Globals

uint64 BaseAddress;

FILE* NewFile = NULL;

std::vector<UObject*> Members;
std::vector<Stats*> MembersStats;

UObject* Leader = nullptr;
UObject* Aerith = nullptr;

std::string CurrentAbilityName;

uint32 MembersNum = 0;
uint32 CurrentMemberMoveToIndex = 0;
uint32 CurrentMemberCheckHPIndex = 0;
uint32 LimitTimesNum = 0;
uint32 SetLimitTimes = 0;

FVector AerithLocation;

Stats* AerithStats = nullptr;

float AerithATBCharge;
float LimitToAdd;
float CloudSavedLimit;
float BarretSavedLimit;
float TifaSavedLimit;
float AerithSavedLimit;

bool bConsole = true;
bool bDebugFile = true;

bool bArcaneWardActive = true;
bool bATBWardActive = true;
bool bLustrousShieldActive = true;
bool bTransferActive = true;
bool bLimitTransferActive = true;

bool bDoOnceEntryBattle = true;
bool bSetLimit = false;
bool bLimitsSaved = false;
bool bAvoidFirstTimeLimit = true;
bool bInBattle = false;
bool bGiveMoveToOrder = false;
bool bAerithATBSetted = false;

// Functions Declarations

typedef void(*CFunction)(UObject*, FFrame&, uint8*, UFunction*);
CFunction OgCFunction = nullptr;
uint64 CallFunction;

typedef void(*CFBName)(UObject*, const wchar_t*, uint64_t, UObject*, bool);
CFBName OgCFBName = nullptr;
uint64 CallFunctionByNameWithArguments;

typedef void(*PInternal)(UObject*, FFrame&, uint8*);
PInternal OgPInternal = nullptr;
uint64 ProcessInternal;

typedef void(*PrEvent)(UObject*, UFunction*, void*);
PrEvent OgPrEvent = nullptr;
uint64 ProcessEventDeref;

// Functions

Stats* GetStats(std::string CharacterName)
{
	uint64 ptr7modifier = 0;

	if (CharacterName.find("Cloud") != std::string::npos)
	{
		ptr7modifier = 0x20;
	}
	else if (CharacterName.find("Barret") != std::string::npos)
	{
		ptr7modifier = 0x60;
	}
	else if (CharacterName.find("Tifa") != std::string::npos)
	{
		ptr7modifier = 0xA0;
	}
	else if (CharacterName.find("Aerith") != std::string::npos)
	{
		ptr7modifier = 0xE0;
	}

	uint64 ptr1 = BaseAddress + 0x579DAE8;
	uint64 ptr2 = *(uint64*)ptr1;
	uint64 ptr3 = ptr2 + 0x8;
	uint64 ptr4 = *(uint64*)ptr3;
	uint64 ptr5 = ptr4 + 0x8;
	uint64 ptr6 = *(uint64*)ptr5;
	uint64 ptr7 = ptr6 + ptr7modifier;

	return (Stats*)ptr7;
}

Stats* GetStats(UObject* Character)
{
	uint64 ptr7modifier = 0;

	if (Character->GetName().find("Cloud") != std::string::npos)
	{
		ptr7modifier = 0x20;
	}
	else if (Character->GetName().find("Barret") != std::string::npos)
	{
		ptr7modifier = 0x60;
	}
	else if (Character->GetName().find("Tifa") != std::string::npos)
	{
		ptr7modifier = 0xA0;
	}
	else if (Character->GetName().find("Aerith") != std::string::npos)
	{
		ptr7modifier = 0xE0;
	}

	uint64 ptr1 = BaseAddress + 0x579DAE8;
	uint64 ptr2 = *(uint64*)ptr1;
	uint64 ptr3 = ptr2 + 0x8;
	uint64 ptr4 = *(uint64*)ptr3;
	uint64 ptr5 = ptr4 + 0x8;
	uint64 ptr6 = *(uint64*)ptr5;
	uint64 ptr7 = ptr6 + ptr7modifier;

	return (Stats*)ptr7;
}
