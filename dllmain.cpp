#include "Decl.h"
#include "MinHook.h"
#include <Psapi.h>
#include <iostream>
#include <filesystem>
#include <fstream>

// Functions Hooking

void HookedCallFunction(UObject* Obj, FFrame& Frame, uint8* Result, UFunction* Function)
{
	if (bSetLimit)
	{
		if (!bAvoidFirstTimeLimit && SetLimitTimes < 60)
		{
			Stats* CloudStats = GetStats("Cloud");
			Stats* BarretStats = GetStats("Barret");
			Stats* TifaStats = GetStats("Tifa");
			Stats* AerithStats = GetStats("Aerith");
			CloudStats->LimitGauge = CloudSavedLimit;
			BarretStats->LimitGauge = BarretSavedLimit;
			TifaStats->LimitGauge = TifaSavedLimit;
			AerithStats->LimitGauge = AerithSavedLimit;

			printf("Loaded Limits %f %f %f %f\n", CloudSavedLimit, BarretSavedLimit, TifaSavedLimit, AerithSavedLimit);
		}
		else
		{
			SetLimitTimes = 0;
			bSetLimit = false;
		}

		if (bSetLimit)
			SetLimitTimes++;

		bAvoidFirstTimeLimit = false;
	}
	
	if (Function->GetFullName().find("Function EndGame.EndPartyAPI.GetPartyMembers") != std::string::npos)
	{
		if (MembersNum == 0)
		{
			OgCFunction(Obj, Frame, Result, Function);

			if (*(uint8**)Result)
			{
				TArray<UObject*> Temp = *(TArray<UObject*>*)(UObject***)Result;
				MembersNum = Temp.Num;

				for (int i = 0; i < MembersNum; i++)
				{
					Members.push_back(Temp.Data[i]);
					MembersStats.push_back(GetStats(Temp.Data[i]));
					printf("Party Member #%i: %s\n", i, Temp.Data[i]->GetName().c_str());
				}	
			}

			bInBattle = true;
			bLimitsSaved = false;

			return;
		}
	}
	
	if (bGiveMoveToOrder)
	{
		if (!bAerithATBSetted)
		{
			AerithStats = GetStats("Aerith");
			bAerithATBSetted = true;
			printf("Aerith ATB Setted\n");
		}
		
		if ((CurrentAbilityName == "PC0003_00_Burst006" || AerithStats->ATB > 999) && CurrentMemberMoveToIndex < MembersNum)
		{
			if (Members[CurrentMemberMoveToIndex] && Members[CurrentMemberMoveToIndex] != Aerith)
			{
				{
					UFunction* fn = (UFunction*)ObjObjects->FindObject("Function Engine.Actor.K2_GetActorLocation");
					struct {
						FVector ReturnValue;
					} parms;

					OgPrEvent(Aerith, fn, &parms);

					AerithLocation = parms.ReturnValue;
				}

				{
					UFunction* fn = (UFunction*)ObjObjects->FindObject("Function Engine.Actor.K2_SetActorLocation");

					struct {
						FVector NewLocation;
						char x[0x8C];
					} parms;

					parms = { AerithLocation };

					OgPrEvent(Members[CurrentMemberMoveToIndex], fn, &parms);
				}

				printf("%s teleported\n", Members[CurrentMemberMoveToIndex]->GetName().c_str());
			}

			CurrentMemberMoveToIndex++;
		}
		else
		{
			Stats* Stats = GetStats("Aerith");
			AerithStats->ATB = 0.f;
			CurrentMemberMoveToIndex = 0;
			bGiveMoveToOrder = false;
			printf("Ended teleport\n");
		}
	}
	
	if (bInBattle)
	{	
		if (CurrentMemberCheckHPIndex < MembersNum)
		{
			if (MembersStats[CurrentMemberCheckHPIndex] && MembersStats[CurrentMemberCheckHPIndex]->InfHP == MembersStats[CurrentMemberCheckHPIndex]->MaxHP)
			{
				MembersStats[CurrentMemberCheckHPIndex]->InfHP -= 1;
				printf("%s HP DOWN\n", Members[CurrentMemberCheckHPIndex]->GetName().c_str());
			}

			CurrentMemberCheckHPIndex++;
		}
		else
		{
			CurrentMemberCheckHPIndex = 0;
		}
	}
	
	OgCFunction(Obj, Frame, Result, Function);
}

void HookedCallFunctionByNameWithArguments(UObject* Obj, const wchar_t* Str, uint64_t Ar, UObject* Executor, bool bForceCallWithNonExec)
{
	if (std::wstring(Str) == L"lol")
	{
		UFunction* DoLoadOg = (UFunction*)ObjObjects->FindObject("Function GameplayTasks.GameplayTask_SpawnActor.SpawnActor");
		printf("%p %p\n", DoLoadOg, BaseAddress);
	}
	// OgCFBName(Obj, Str, Ar, Executor, bForceCallWithNonExec);
}

void HookedProcessInternal(UObject* Obj, FFrame& Frame, uint8* Result)
{
	OgPInternal(Obj, Frame, Result);
}

void HookedProcessEvent(UObject* Obj, UFunction* Function, void* Parms)
{
	if (Function->GetFullName().find("OnDead") != std::string::npos)
	{
		{
			UFunction* fn = (UFunction*)ObjObjects->FindObject("Function EndGame.EndBattleAIController.GetSceneEnemyCount");
			struct {
				uint32 ReturnValue;
			} parms;

			OgPrEvent(Obj, fn, &parms);

			printf("EnemyCount: %i\n", parms.ReturnValue);

			if (parms.ReturnValue == 0)
			{
				Stats* CloudStats = GetStats("Cloud");
				CloudSavedLimit = CloudStats->LimitGauge;

				Stats* BarretStats = GetStats("Barret");
				BarretSavedLimit = BarretStats->LimitGauge;

				Stats* TifaStats = GetStats("Tifa");
				TifaSavedLimit = TifaStats->LimitGauge;

				Stats* AerithStats = GetStats("Aerith");
				AerithSavedLimit = AerithStats->LimitGauge;

				printf("Saved Limits %f %f %f %f\n", CloudSavedLimit, BarretSavedLimit, TifaSavedLimit, AerithSavedLimit);

				bLimitsSaved = true;
			}
		}
	}

	if (Function->GetFullName().find("OnEntryBattleScene") != std::string::npos)
	{
		if (bDoOnceEntryBattle)
		{ 
			bSetLimit = true;
			bDoOnceEntryBattle = false;
		}
	}

	if (Function->GetFullName().find("OnExitBattleScene") != std::string::npos)
	{
		if (!bLimitsSaved)
		{
			Stats* CloudStats = GetStats("Cloud");
			CloudSavedLimit = CloudStats->LimitGauge;

			Stats* BarretStats = GetStats("Barret");
			BarretSavedLimit = BarretStats->LimitGauge;

			Stats* TifaStats = GetStats("Tifa");
			TifaSavedLimit = TifaStats->LimitGauge;

			Stats* AerithStats = GetStats("Aerith");
			AerithSavedLimit = AerithStats->LimitGauge;

			printf("Saved Limits %f %f %f %f\n", CloudSavedLimit, BarretSavedLimit, TifaSavedLimit, AerithSavedLimit);

			bLimitsSaved = true;
		}

		bDoOnceEntryBattle = true;
		bSetLimit = false;
		bGiveMoveToOrder = false;
		bInBattle = false;
		Leader = nullptr;
		Aerith = nullptr;
		Members.clear();
		MembersStats.clear();
		MembersNum = 0;
		CurrentMemberCheckHPIndex = 0;
		CurrentMemberMoveToIndex = 0;
		printf("Reset for battle exit\n");
	}
	
	if (Function->GetFullName().find("Function EndGame.EndBattleAIController.OnGiveDamage") != std::string::npos)
	{
		FName* AbilityFName = (FName*)Parms + 0x02;
		
		if (((AbilityFName->GetName().find("it_potion") != std::string::npos || AbilityFName->GetName().find("IT_potion") != std::string::npos) && bTransferActive) ||
			((AbilityFName->GetName().find("it_hpotion") != std::string::npos || AbilityFName->GetName().find("IT_hpotion") != std::string::npos) && bLimitTransferActive))
		{
			uint8* ptr = (uint8*)Parms + 0x08;
			UObject* TargetCharacter = *(UObject**)ptr;
			UObject* CauserController = Obj;
			UObject* CauserCharacter = nullptr;

			{
				UFunction* fn = (UFunction*)ObjObjects->FindObject("Function EndGame.EndBattleAIController.GetCharacter");
				struct {
					UObject* ReturnValue;
				} parms;

				OgPrEvent(CauserController, fn, &parms);

				CauserCharacter = parms.ReturnValue;
			}

			if ((AbilityFName->GetName().find("it_potion") != std::string::npos || AbilityFName->GetName().find("IT_potion") != std::string::npos)
				&& CauserCharacter != TargetCharacter)
			{
				Stats* TargetCharacterStats = GetStats(TargetCharacter);
				TargetCharacterStats->ATB = std::clamp(TargetCharacterStats->ATB + 500.f, 0.f, 2000.f);
				printf("Added 500 ATB to %s\n", TargetCharacter->GetName().c_str());
			}

			if ((AbilityFName->GetName().find("it_hpotion") != std::string::npos || AbilityFName->GetName().find("IT_hpotion") != std::string::npos)
				&& CauserCharacter != TargetCharacter)
			{
				LimitTimesNum++;
				Stats* CauserCharacterStats = GetStats(CauserCharacter);
				
				if (LimitTimesNum == 1)
				{
					LimitToAdd = CauserCharacterStats->LimitGauge / 2;
				}

				CauserCharacterStats->LimitGauge = 0;


				Stats* TargetCharacterStats = GetStats(TargetCharacter);
				TargetCharacterStats->LimitGauge = std::clamp(TargetCharacterStats->LimitGauge + LimitToAdd, 0.f, 1350.f);

				printf("Added %f LIMIT to %s\n", LimitToAdd, TargetCharacter->GetName().c_str());

				if (LimitTimesNum == 2)
				{ 
					LimitTimesNum = 0;
				}
			}
		}
	}
	
	if (Obj->GetFullName().find("Party_Coordinator_C Party_Coordinator.Party_Coordinator.PersistentLevel.Party_Coordinator_C_1") != std::string::npos)
	{
		if (!Leader && MembersNum > 0)
		{
			UFunction* fn = (UFunction*)ObjObjects->FindObject("Function EndGame.EndPartyAPI.GetPartyLeader");
			struct {
				UObject* ReturnValue;
			} parms;

			OgPrEvent(Obj, fn, &parms);

			Leader = parms.ReturnValue;

			printf("Leader is %s\n", Leader->GetName().c_str());
		}
	}
	
	if (Function->GetFullName().find("Function EndGame.EndBattleAIController.OnTakeAbilityInvoke") != std::string::npos)
	{
		UObject* CauserChara = *(UObject**)Parms;
		FName* AbilityFName = (FName*)Parms + 0x01;
		std::string AbilityName = AbilityFName->GetName();
		
		if ((AbilityName == "PC0003_00_Burst002" && bArcaneWardActive) ||
			(AbilityName == "PC0003_00_Burst004" && bLustrousShieldActive) ||
			(AbilityName == "PC0003_00_Burst006" && bATBWardActive))
		{
			Aerith = CauserChara;
			CurrentAbilityName = AbilityName;
			bGiveMoveToOrder = true;
			printf("MOVE!\n");
		}
	}

	if (Function->GetFullName().find("Function EndGame.EndBattleAIController.OnChangeLeader") != std::string::npos)
	{
		Leader = *(UObject**)Parms;
		printf("Leader changed: %s\n", Leader->GetName().c_str());
	}
	
	OgPrEvent(Obj, Function, Parms);
}

// Initialize Hooks

void InitHooks()
{
	MODULEINFO miGame = { NULL };
	HMODULE hmModule = GetModuleHandle(NULL);

	GetModuleInformation(GetCurrentProcess(), hmModule, &miGame, sizeof(MODULEINFO));

	BaseAddress = (uint64)miGame.lpBaseOfDll;

	NamePoolData = (FNamePool*)(BaseAddress + 0x5934E80);
	ObjObjects = (TUObjectArray*)(BaseAddress + 0x53823D0);

	if (MH_Initialize() != MH_OK)
	{
		printf("Failed to Initialize\n");
		return;
	}

	// CallFunction Hook

	if (true)
	{
		CallFunction = BaseAddress + 0x1E8A1D0;
		OgCFunction = (CFunction)CallFunction;

		if (MH_CreateHook((LPVOID)CallFunction, &HookedCallFunction,
			reinterpret_cast<LPVOID*>(&OgCFunction)) != MH_OK)
		{
			printf("Failed to Hook CallFunction\n");
			return;
		}

		if (MH_EnableHook((LPVOID)CallFunction) != MH_OK)
		{
			printf("Failed to Enable Hook CallFunction\n");
			return;
		}

		printf("CallFunction Hooked!\n");
	}

	// CallFunctionByNameWithArguments Hook

	if (true)
	{
		CallFunctionByNameWithArguments = BaseAddress + 0x1E8A910;
		OgCFBName = (CFBName)CallFunctionByNameWithArguments;

		if (MH_CreateHook((LPVOID)CallFunctionByNameWithArguments, &HookedCallFunctionByNameWithArguments,
			reinterpret_cast<LPVOID*>(&OgCFBName)) != MH_OK)
		{
			printf("Failed to Hook CallFunctionByNameWithArguments\n");
			return;
		}

		if (MH_EnableHook((LPVOID)CallFunctionByNameWithArguments) != MH_OK)
		{
			printf("Failed to Enable Hook CallFunctionByNameWithArguments\n");
			return;
		}

		printf("CallFunctionByNameWithArguments Hooked!\n");
	}

	// ProcessInternal Hook

	if (false)
	{
		ProcessInternal = BaseAddress + 0x1E8A770;
		OgPInternal = (PInternal)ProcessInternal;

		if (MH_CreateHook((LPVOID)ProcessInternal, &HookedProcessInternal,
			reinterpret_cast<LPVOID*>(&OgPInternal)) != MH_OK)
		{
			printf("Failed to Hook ProcessInternal\n");
			return;
		}

		if (MH_EnableHook((LPVOID)ProcessInternal) != MH_OK)
		{
			printf("Failed to Enable Hook ProcessInternal\n");
			return;
		}

		printf("ProcessInternal Hooked!\n");
	}

	// ProcessEvent Hook

	if (true)
	{
		uint64 vTableOffset = 0x536B310;
		uint64 vTable = BaseAddress + vTableOffset;
		uint64 vTableDeref = *(uint64*)vTable;
		uint64 ProcessEventOffset = 0x220;
		const uint64 ProcessEvent = vTableDeref + ProcessEventOffset;
		ProcessEventDeref = *(uint64*)ProcessEvent;
		OgPrEvent = (PrEvent)ProcessEventDeref;

		if (MH_CreateHook((LPVOID)ProcessEventDeref, &HookedProcessEvent,
			reinterpret_cast<LPVOID*>(&OgPrEvent)) != MH_OK)
		{
			printf("Failed to Hook ProcessEvent\n");
			return;
		}

		if (MH_EnableHook((LPVOID)ProcessEventDeref) != MH_OK)
		{
			printf("Failed to Enable Hook ProcessEvent\n");
			return;
		}

		printf("ProcessEvent Hooked!\n");
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		if (bConsole)
		{
			AllocConsole();
			FILE* fDummy;
			freopen_s(&fDummy, "CONIN$", "r", stdin);
			freopen_s(&fDummy, "CONOUT$", "w", stderr);
			freopen_s(&fDummy, "CONOUT$", "w", stdout);
		}	

		if (bDebugFile)
		{
			auto err = fopen_s(&NewFile, "funcs.txt", "w+");
		}

		{
			using namespace std::filesystem;

			path CurrentPath = current_path();

			for (const auto& File : directory_iterator(CurrentPath))
			{
				std::string FileName = path(File).filename().string();
				
				if (FileName.find("AbilityModSave") != std::string::npos)
				{
					std::fstream Stream;
					std::string Line;
					Stream.open(File);

					for (int i = 0; i < 5; i++)
					{
						if (Line.find("ArcaneWard ") != std::string::npos)
						{
							bArcaneWardActive = stoi(Line.substr(11, 1));
						}

						if (Line.find("ATBWard ") != std::string::npos)
						{
							bATBWardActive = stoi(Line.substr(8, 1));
						}

						if (Line.find("LustrousShield ") != std::string::npos)
						{
							bLustrousShieldActive = stoi(Line.substr(15, 1));
						}

						if (Line.find("Transfer ") != std::string::npos)
						{
							bTransferActive = stoi(Line.substr(9, 1));
						}

						if (Line.find("LimitTransfer ") != std::string::npos)
						{
							bLimitTransferActive = stoi(Line.substr(14, 1));
						}
					}
				}
			}
		}

		InitHooks();
	}

	return true;
}

