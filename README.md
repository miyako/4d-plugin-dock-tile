4d-plugin-dock-tile
===================

This plugins allows 4D to control the dock tile.

##Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|🆗|🆗|🚫|🚫|

Commands
---

```c
// --- Dock
DOCK_Get_badge
DOCK_SET_BADGE
DOCK_CANCEL_BOUNCE
DOCK_Bounce
DOCK_SET_ICON
DOCK_Get_icon

// --- Size
DOCK_GET_SIZE
DOCK_GET_SCREEN_FRAME
```

Example
-------
To resize the window to touch the Dock.

```
DOCK GET SCREEN FRAME ($x;$y;$width;$height)

$offsetY:=102  //Design Mode Tool Bar

SET WINDOW RECT($x;$offsetY;$x+$width;$height+Menu bar height;Frontmost window)
```
