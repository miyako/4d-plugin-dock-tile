![version](https://img.shields.io/badge/version-18%2B-EB8E5F)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-dock-tile)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-dock-tile/total)

# 4d-plugin-dock-tile

Control the dock tile.

### Commands

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

### Remarks

Badge notofication can be disabled in system settings.

<img width="668" alt="badge" src="https://user-images.githubusercontent.com/1725068/92133614-8b915b00-ee43-11ea-96e3-ab222be0fb72.png">

The badge is automatically removed when the plugin is unloaded.

### Examples

To resize the window to touch the Dock.

```
DOCK GET SCREEN FRAME ($x;$y;$width;$height)

$offsetY:=102  //Design Mode Tool Bar

SET WINDOW RECT($x;$offsetY;$x+$width;$height+Menu bar height;Frontmost window)
```
