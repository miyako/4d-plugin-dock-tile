4d-plugin-dock-tile
===================

This plugins allows 4D to control the dock tile.

4D Cocoa OS X 10.8+(32/64)  
4D Carbon OS X 10.6+ (32)

Example
-------
To resize the window to touch the Dock.

```
DOCK GET SCREEN FRAME ($x;$y;$width;$height)

$offsetY:=102  //v13

SET WINDOW RECT($x;$offsetY;$x+$width;$height+Menu bar height;Frontmost window)
```
