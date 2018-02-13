scriptname GlobalWorkshop:MasterScript extends Quest

CustomEvent WorkshopChangedEvent

group AutoFill
    WorkshopParentScript property WorkshopParent auto const mandatory
endgroup

WorkshopScript lastUsedWorkshopRef = none

string userLogName = "GlobalWorkshop" const

function gwsOpenUserLog() debugonly
    Debug.OpenUserLog(userLogName)
endfunction
function gwsTrace(string asTextToPrint, int aiSeverity = 0) debugonly
    Debug.TraceUser(userLogName, asTextToPrint, aiSeverity)
endfunction
function gwsTraceSelf(string asFunctionName, string asTextToPrint, int aiSeverity = 0) debugonly
    gwsTrace(self + "-->" + asFunctionName + "(): " + asTextToPrint, aiSeverity)
endfunction
function gwsTraceConditional(string asTextToPrint, bool abShowTrace, int aiSeverity = 0) debugonly
    if (abShowTrace)
        gwsTrace(asTextToPrint, aiSeverity)
    endif
endfunction
function gwsTraceSelfConditional(string asFunctionName, string asTextToPrint, bool abShowTrace, int aiSeverity = 0) debugonly
    if (abShowTrace)
        gwsTraceSelf(asFunctionName, asTextToPrint, aiSeverity)
    endif
endfunction

function DispatchWorkshopChangedEvent(WorkshopScript workshopRef)
    if (workshopRef && workshopRef.OwnedByPlayer && workshopRef != lastUsedWorkshopRef)
        GlobalWorkshop.SwapWorkshop(workshopRef, lastUsedWorkshopRef)

        ;float x = lastUsedWorkshopRef.GetPositionX()
        ;float y = lastUsedWorkshopRef.GetPositionY()
        ;float z = lastUsedWorkshopRef.GetPositionZ()
        lastUsedWorkshopRef.MoveTo(workshopRef, afZOffset = -1024.0);, abMatchRotation = false)
        lastUsedWorkshopRef.RemoveAllItems(workshopRef, true)
        lastUsedWorkshopRef.MoveToMyEditorLocation()
        ;lastUsedWorkshopRef.SetPosition(x, y, z)

        WorkshopScript oldWorkshopRef = lastUsedWorkshopRef
        lastUsedWorkshopRef = workshopRef

        var[] args = new var[2]
        args[0] = workshopRef
        args[1] = oldWorkshopRef
        self.SendCustomEvent("WorkshopChangedEvent", args)

        gwsTraceSelf("DispatchWorkshopChangedEvent", "Workshop changed from " + oldWorkshopRef + " to " + workshopRef + ".")
    endif
endfunction

event OnQuestInit()
    self.RegisterForCustomEvent(WorkshopParent, "WorkshopPlayerOwnershipChanged")
    Actor player = Game.GetPlayer()
    self.RegisterForRemoteEvent(player, "OnLocationChange")
    self.RegisterForRemoteEvent(player, "OnPlayerLoadGame")
endevent
;/
event OnQuestShutdown()
endevent
/;

event Actor.OnPlayerLoadGame(Actor akSender)
    gwsOpenUserLog()
endevent

event WorkshopParentScript.WorkshopPlayerOwnershipChanged(WorkshopParentScript akSender, Var[] akArgs)
	bool bOwnedByPlayer = akArgs[0] as bool
	WorkshopScript workshopRef = akArgs[1] as WorkshopScript

	; Handle workshop ownership changes.
	if (bOwnedByPlayer)
        gwsTraceSelf("WorkshopParentScript.WorkshopPlayerOwnershipChanged", "Ownership of " + workshopRef + " gained.")
        DispatchWorkshopChangedEvent(workshopRef)
	endif
endevent
event Actor.OnLocationChange(Actor akSender, Location akOldLoc, Location akNewLoc)
    ;Debug.MessageBox("Entered new location: " + akNewLoc.GetName())
    ObjectReference workshopRef = GlobalWorkshop.FindWorkshop(akNewLoc)
    if (workshopRef)
        gwsTraceSelf("Actor.OnLocationChange", "Successfully found workshop: " + workshopRef)
        DispatchWorkshopChangedEvent(workshopRef as WorkshopScript)
    endif
endevent
