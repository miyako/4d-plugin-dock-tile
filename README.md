# 4d-plugin-dock-tile
===================

Control the dock tile.

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
||<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" /> |<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" /> |<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />

### Version

<img src="https://user-images.githubusercontent.com/1725068/41266195-ddf767b2-6e30-11e8-9d6b-2adf6a9f57a5.png" width="32" height="32" />

![Apple-Notarized-App-Service-logo-icon](https://user-images.githubusercontent.com/1725068/73900022-bf1dfe00-48d1-11ea-81a7-b715a8e7206c.jpg)

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
