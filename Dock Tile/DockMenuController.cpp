//
//  DockMenuController.cpp
//  Dock Tile
//
//  Created by Eric Marchand on 27/07/2021.
//

#include "DockMenuController.h"
#include "C_TEXT.h"
#import <objc/objc-class.h>

namespace ExecuteInProcess
{
    PA_Unichar* methodName;
    void call()
    {
        PA_long32 methodID = PA_GetMethodID(methodName);
        PA_Variable parameters[0];
        PA_ExecuteMethodByID(methodID, parameters, 0);
        // or
        // PA_Unistring key = PA_CreateUnistring(&methodName[0]);
        // PA_ExecuteMethod(&key);
    }
}

@implementation DockMenuController

@synthesize menu;

+ (id)shared
{
    static DockMenuController *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
        sharedInstance.menu = [[NSMenu alloc] initWithTitle:@"dock app menu"];
        [sharedInstance.menu setAutoenablesItems: NO];
        dispatch_async(dispatch_get_main_queue(), ^{
            Method delegateMethod = class_getInstanceMethod([sharedInstance class], @selector(applicationDockMenu:));
            IMP imp = method_getImplementation(delegateMethod);
            class_addMethod([[[NSApplication sharedApplication] delegate] class], @selector(applicationDockMenu:), imp, method_getTypeEncoding(delegateMethod));
        });
    });
    return sharedInstance;
}

- (void) clearAll
{
    [menu removeAllItems];
}

- (void) add:(DockMenuItem)item
{
    NSMenuItem *menuItem = [[NSMenuItem alloc] initWithTitle: item.title action: @selector(menuItemAction:) keyEquivalent:@""];
    [menuItem setEnabled: YES];
    [menuItem setTarget: [DockMenuController shared]];
    [menuItem setRepresentedObject: item.methodName];
    dispatch_async(dispatch_get_main_queue(), ^{
        [menu addItem: menuItem];
    });
}

- (void) menuItemAction:(NSMenuItem *)sender
{
    NSString* methodName = [sender representedObject];
    NSLog(@"Dock Menu Item Action %@", methodName);

    // schedule it in a process that could execute 4d code
    C_TEXT methodNameText;
    methodNameText.setUTF16String(methodName);
    ExecuteInProcess::methodName = const_cast<PA_Unichar*>(methodNameText.getUTF16StringPtr()); // /!\ not very safe if multiple call
    PA_NewProcess((void *) ExecuteInProcess::call, 0, (PA_Unichar *)"$\0d$\0o\0c\0k\0m\0e\0n\0u\0");
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
    if([[[DockMenuController shared] menu] numberOfItems]>0)
    {
        return [[DockMenuController shared] menu];
    }
    return nil;
}

@end
