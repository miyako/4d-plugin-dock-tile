//
//  DockMenuController.h
//  Dock Tile
//
//  Created by Eric Marchand on 27/07/2021.
//

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#import <Cocoa/Cocoa.h>

typedef struct DockMenuItem {
    NSString* title;
    NSString* methodName;
} DockMenuItem;

@interface DockMenuController : NSObject

@property (atomic, assign) NSMenu* menu;

+ (id)shared;

- (void) clearAll;
- (void) add:(DockMenuItem)item;

- (void) menuItemAction:(NSMenuItem *)sender;
- (NSMenu *)applicationDockMenu:(NSApplication *)sender;

@end

#endif /* MENU_CONTROLLER_H */
