#include "f4se/PluginAPI.h"
#include "f4se/GameAPI.h"
#include "f4se_common/f4se_version.h"

#include "f4se/GameReferences.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"

IDebugLog gLog("GlobalWorkshop.log");

struct {
	PluginHandle pluginHandle = kPluginHandle_Invalid;
	F4SEPapyrusInterface* papyrus = nullptr;
} global;

TESObjectREFR* FindWorkshop(StaticFunctionTag*, BGSLocation* location) {
	_MESSAGE("%s", __FUNCSIG__);

	if (location) {
		BGSLocationRefType* kWorkshopRefType = (BGSLocationRefType*)LookupFormByID(0x000234e9);
		for (UInt32 i = 0; i < location->staticReferences.count; ++i) {
			if (location->staticReferences[i].refType == kWorkshopRefType) {
				return DYNAMIC_CAST(LookupFormByID(location->staticReferences[i].markerForm), TESForm, TESObjectREFR);
			}
		}
	}
	return nullptr;
}

void SwapWorkshop(StaticFunctionTag*, TESObjectREFR* newWorkshop, TESObjectREFR* oldWorkshop) {
	_MESSAGE("%s", __FUNCSIG__);

	if (newWorkshop && oldWorkshop && newWorkshop != oldWorkshop) {
		if (newWorkshop->extraDataList && oldWorkshop->extraDataList) {
			_MESSAGE("Both references have extra data list.");

			BSExtraData* newWorkshopData = newWorkshop->extraDataList->GetByType(kExtraData_WorkshopExtraData);
			BSExtraData* oldWorkshopData = oldWorkshop->extraDataList->GetByType(kExtraData_WorkshopExtraData);

			if (newWorkshopData && oldWorkshopData) {
				newWorkshop->extraDataList->Remove(kExtraData_WorkshopExtraData, newWorkshopData);
				oldWorkshop->extraDataList->Remove(kExtraData_WorkshopExtraData, oldWorkshopData);
				newWorkshop->extraDataList->Add(kExtraData_WorkshopExtraData, oldWorkshopData);
				oldWorkshop->extraDataList->Add(kExtraData_WorkshopExtraData, newWorkshopData);
			}
			else if (!newWorkshopData && oldWorkshopData) {
				oldWorkshop->extraDataList->Remove(kExtraData_WorkshopExtraData, oldWorkshopData);
				newWorkshop->extraDataList->Add(kExtraData_WorkshopExtraData, oldWorkshopData);
			} /*
			else if (newWorkshopData && !oldWorkshopData) {
				// Do nothing.
			} */
		}
	}
}

bool RegisterPapyrusFunctions(VirtualMachine* vm) {
	_MESSAGE("RegisterPapyrusFunctions begin");

	constexpr char* papyrusClassName = "GlobalWorkshop";

	vm->RegisterFunction(new NativeFunction1<StaticFunctionTag, TESObjectREFR*, BGSLocation*>("FindWorkshop", papyrusClassName, FindWorkshop, vm));
	vm->RegisterFunction(new NativeFunction2<StaticFunctionTag, void, TESObjectREFR*, TESObjectREFR*>("SwapWorkshop", papyrusClassName, SwapWorkshop, vm));

	_MESSAGE("RegisterPapyrusFunctions end");

	return true;
}

/* Plugin Query */

extern "C" {
	bool F4SEPlugin_Query(const F4SEInterface* f4se, PluginInfo* info) {
		_MESSAGE("F4SEPlugin_Query begin");

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Global Workshop";
		info->version = 1;

		// store plugin handle so we can identify ourselves later
		global.pluginHandle = f4se->GetPluginHandle();

		if (f4se->isEditor) {
			_MESSAGE("\tloaded in editor, marking as incompatible");

			return false;
		}
		else if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_64) {
			_MESSAGE("\tunsupported runtime version %08X", f4se->runtimeVersion);

			return false;
		}

		global.papyrus = (F4SEPapyrusInterface*)f4se->QueryInterface(kInterface_Papyrus);
		if (!global.papyrus) {
			_MESSAGE("\tcouldn't get papyrus interface");

			return false;
		}
		if (global.papyrus->interfaceVersion < F4SEPapyrusInterface::kInterfaceVersion) {
			_MESSAGE("\tpapyrus interface too old (%d expected %d)", global.papyrus->interfaceVersion, F4SEPapyrusInterface::kInterfaceVersion);

			return false;
		}

		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		_MESSAGE("F4SEPlugin_Query end");

		// supported runtime version
		return true;
	}

	bool F4SEPlugin_Load(const F4SEInterface * f4se) {
		_MESSAGE("F4SEPlugin_Load begin");

		global.papyrus->Register(RegisterPapyrusFunctions);

		_MESSAGE("F4SEPlugin_Load end");

		return true;
	}

};
