/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.c
 #	source generated by 4D Plugin Wizard
 #	Project : Dock Tile
 #	author : miyako
 #	2013/02/18
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(int32_t selector, PA_PluginParameters params)
{
	try
	{
		int32_t pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (int32_t pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
			// --- App
			
		case 1 :
			APP_HIDE(pResult, pParams);
			break;
			
		case 2 :
			APP_SHOW(pResult, pParams);
			break;
			
		case 3 :
			APP_HIDE_OTHERS(pResult, pParams);
			break;
			
		case 4 :
			APP_SHOW_OTHERS(pResult, pParams);
			break;
			
		case 5 :
			APP_Is_hidden(pResult, pParams);
			break;
			
			// --- Dock
			
		case 6 :
			DOCK_Get_badge(pResult, pParams);
			break;
			
		case 7 :
			DOCK_SET_BADGE(pResult, pParams);
			break;
			
		case 8 :
			DOCK_CANCEL_BOUNCE(pResult, pParams);
			break;
			
		case 9 :
			DOCK_Bounce(pResult, pParams);
			break;
			
		case 10 :
			DOCK_SET_ICON(pResult, pParams);
			break;
			
		case 11 :
			DOCK_Get_icon(pResult, pParams);
			break;
			
	}
}

// ------------------------------------- Dock -------------------------------------

void DOCK_Get_badge(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT returnValue;
	
	returnValue.setUTF16String([[[NSApplication sharedApplication]dockTile]badgeLabel]);
	returnValue.setReturn(pResult);
}

void DOCK_SET_BADGE(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_TEXT Param1;

	Param1.fromParamAtIndex(pParams, 1);

	NSString *badgeLabel = Param1.copyUTF16String();
	[[[NSApplication sharedApplication]dockTile] setBadgeLabel:badgeLabel];
	[badgeLabel release];
}

void DOCK_CANCEL_BOUNCE(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;

	Param1.fromParamAtIndex(pParams, 1);

	[[NSApplication sharedApplication]cancelUserAttentionRequest:Param1.getIntValue()];
}

void DOCK_Bounce(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;
	C_LONGINT returnValue;

	Param1.fromParamAtIndex(pParams, 1);

#define CMU_DELAY_PROCESS 323
	
	PA_Variable args[2];
	
	args[0] = PA_CreateVariable(eVK_Longint);
	args[1] = PA_CreateVariable(eVK_Longint);
	
	PA_SetLongintVariable(&args[0], PA_GetCurrentProcessNumber());	
	PA_SetLongintVariable(&args[1], 1);	
	PA_ExecuteCommandByID(CMU_DELAY_PROCESS, args, 2);
	
	returnValue.setIntValue([[NSApplication sharedApplication]requestUserAttention:Param1.getIntValue()]);
	returnValue.setReturn(pResult);
}

void DOCK_SET_ICON(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_PICTURE Param1;

	Param1.fromParamAtIndex(pParams, 1);
	NSImage *iconImage =Param1.copyImage();
	
	if(iconImage)
	{
		[[NSApplication sharedApplication]setApplicationIconImage:iconImage];
		[iconImage release];
	}
}

void DOCK_Get_icon(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_PICTURE returnValue;

	NSData *data = [[[NSApplication sharedApplication]applicationIconImage] TIFFRepresentation];
	
	returnValue.setBytes((const uint8_t*)[data bytes], [data length]);
	returnValue.setReturn(pResult);
}

// -------------------------------------- App -------------------------------------

void APP_HIDE(sLONG_PTR *pResult, PackagePtr pParams)
{
	[[NSApplication sharedApplication]hide:nil];	
}

void APP_SHOW(sLONG_PTR *pResult, PackagePtr pParams)
{
	[[NSApplication sharedApplication]unhide:nil];	
}

void APP_HIDE_OTHERS(sLONG_PTR *pResult, PackagePtr pParams)
{
	[[NSApplication sharedApplication]hideOtherApplications:nil];	
}

void APP_SHOW_OTHERS(sLONG_PTR *pResult, PackagePtr pParams)
{
	[[NSApplication sharedApplication]unhideAllApplications:nil];	
}

void APP_Is_hidden(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT returnValue;
	
	returnValue.setIntValue([[NSApplication sharedApplication]isHidden]);	
	returnValue.setReturn(pResult);
}
