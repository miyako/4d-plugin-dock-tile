![version](https://img.shields.io/badge/version-16%2B-8331AE)
![platform](https://img.shields.io/static/v1?label=platform&message=mac-intel%20|%20mac-arm&color=blue)
[![license](https://img.shields.io/github/license/miyako/4d-plugin-dock-tile)](LICENSE)
![downloads](https://img.shields.io/github/downloads/miyako/4d-plugin-dock-tile/total)

# 4d-plugin-dock-tile

Control the Dock tile of the running 4D application on macOS: set or read the badge label, bounce the icon, replace or read the application icon, draw a progress indicator over the icon, and read the Dock tile size and the usable screen area. The plugin drives AppKit's `NSDockTile` and `NSApplication` APIs. Results are `Text`, `Longint`, `Real` and `Picture` values.

Download builds from [Releases](https://github.com/miyako/4d-plugin-dock-tile/releases).

| Command | Returns | Purpose |
|---|---|---|
| [DOCK SET BADGE](#dock-set-badge) | — | Set or clear the badge label on the Dock icon |
| [DOCK Get badge](#dock-get-badge) | `Text` | Read the current badge label |
| [DOCK Bounce](#dock-bounce) | `Longint` (always `0`, see caveat) | Request user attention (bounce the Dock icon) |
| [DOCK CANCEL BOUNCE](#dock-cancel-bounce) | — | Cancel an attention request |
| [DOCK SET ICON](#dock-set-icon) | — | Replace the application icon in the Dock |
| [DOCK Get icon](#dock-get-icon) | `Picture` | Read the current application icon (TIFF) |
| [DOCK SET PROGRESS](#dock-set-progress) | — | Draw a progress bar, circle or squircle over the Dock icon |
| [DOCK GET SIZE](#dock-get-size) | — | Read the Dock tile size |
| [DOCK GET SCREEN FRAME](#dock-get-screen-frame) | — | Read the visible frame of the main screen (without menu bar and Dock) |
| [DOCK MENU](#dock-menu) | — | **Experimental prototype**, do not use in production |

**Platforms:** macOS only (Intel and Apple Silicon). 4D v16 or later.

---

## Requirements & platform notes

- **macOS only.** The plugin bundle contains a `Windows64` binary, but it is the generic Plugin Wizard stub: none of the commands is implemented on Windows.
- **macOS 11.5 or later** for builds from this repository (the Xcode project's deployment target).
- **For 4D v17 and earlier**, move `manifest.json` from `Contents/Resources` to `Contents`.
- **Badges can be disabled by the user.** Badge display depends on the notification settings for the 4D application in System Settings. [DOCK SET BADGE](#dock-set-badge) succeeds silently even when the badge is not shown.

<img width="668" alt="badge" src="https://user-images.githubusercontent.com/1725068/92133614-8b915b00-ee43-11ea-96e3-ab222be0fb72.png">

- **The badge is removed when the plugin is unloaded** (when 4D quits).
- **Main-process execution.** [DOCK SET BADGE](#dock-set-badge), [DOCK Get badge](#dock-get-badge), [DOCK Bounce](#dock-bounce), [DOCK CANCEL BOUNCE](#dock-cancel-bounce), [DOCK SET ICON](#dock-set-icon), [DOCK Get icon](#dock-get-icon), [DOCK GET SIZE](#dock-get-size) and [DOCK GET SCREEN FRAME](#dock-get-screen-frame) run their work in 4D's main process and return when it is done. [DOCK SET PROGRESS](#dock-set-progress) and [DOCK MENU](#dock-menu) schedule their work on the main thread and return immediately.
- **Thread safety.** `manifest.json` declares every command thread-safe. Calling the main-process commands above from a preemptive process has not been verified.
- **No 4D errors.** No command raises a 4D error. Failures are silent: an empty result, or nothing happens.
- **Attention constants.** The plugin defines the constant theme *Dock Bounce Option*:

| Constant | Value |
|---|---|
| `Dock_Bounce_One_Second` | 10 |
| `Dock_Bounce_Indefinite` | 1 |

---

## DOCK SET BADGE

### Syntax

```
DOCK SET BADGE ( label )
```

| Parameter | Type | Description |
|---|---|---|
| `label` | Text | Badge text. Pass `""` to remove the badge |

### Description

Sets the badge label shown on the 4D application's Dock icon and redraws the tile. An empty `label` removes the badge. Any text is accepted; the Dock shows it in a red bubble, usually a number.

### Example

From the plugin's own test method (`TEST.4dm`):

```4d
DOCK SET BADGE ("111")
```

Remove the badge:

```4d
DOCK SET BADGE ("")
```

---

## DOCK Get badge

### Syntax

```
DOCK Get badge → label
```

| Parameter | Type | Description |
|---|---|---|
| Result | Text | The current badge label, or `""` if there is no badge |

### Description

Returns the badge label currently set on the Dock tile, by [DOCK SET BADGE](#dock-set-badge) or by any other code in the 4D process.

### Example

```4d
$label:=DOCK Get badge
If ($label="")
	DOCK SET BADGE ("1")
End if 
```

---

## DOCK Bounce

### Syntax

```
DOCK Bounce ( type ) → result
```

| Parameter | Type | Description |
|---|---|---|
| `type` | Longint | Attention request type. On return, holds the request identifier (see below) |
| Result | Longint | Always `0` (see below) |

### Description

Calls `-[NSApplication requestUserAttention:]` to bounce the 4D icon in the Dock. AppKit only makes the request when 4D is **not** the active application. When 4D is active, nothing happens and the identifier is `0`.

AppKit has two request types: `0` (critical: bounce until the application becomes active) and `10` (informational: bounce once). `Dock_Bounce_One_Second` (10) is the informational request. `Dock_Bounce_Indefinite` is defined as `1`, which is **not** one of AppKit's values, and how AppKit treats `1` has not been verified. For the critical, bounce-until-active behaviour, pass `0`.

**The function result is always `0`.** The plugin writes the request identifier back into the `type` parameter, not into the result. To get the identifier for [DOCK CANCEL BOUNCE](#dock-cancel-bounce), pass a variable and read it after the call (second example).

### Example

From the plugin's own test method (`TEST.4dm`). Because the result is always `0`, `$id` is `0` here and the cancel has no effect:

```4d
  //only when the app is not active
$id:=DOCK Bounce (Dock_Bounce_Indefinite)
DOCK CANCEL BOUNCE ($id)
```

To get a usable identifier, pass a variable:

```4d
C_LONGINT($request)
$request:=Dock_Bounce_One_Second
DOCK Bounce ($request)
  // $request now holds the request identifier (0 if 4D was active)
DOCK CANCEL BOUNCE ($request)
```

---

## DOCK CANCEL BOUNCE

### Syntax

```
DOCK CANCEL BOUNCE ( request )
```

| Parameter | Type | Description |
|---|---|---|
| `request` | Longint | Request identifier, as written back by [DOCK Bounce](#dock-bounce) |

### Description

Calls `-[NSApplication cancelUserAttentionRequest:]` with `request`. A critical request also stops by itself when 4D becomes active, so this command is mostly useful to stop bouncing while 4D is still in the background.

### Example

See [DOCK Bounce](#dock-bounce).

---

## DOCK SET ICON

### Syntax

```
DOCK SET ICON ( icon )
```

| Parameter | Type | Description |
|---|---|---|
| `icon` | Picture | New application icon |

### Description

Converts `icon` to a native image and sets it as the application icon image (`-[NSApplication setApplicationIconImage:]`). This changes the icon of the running 4D process only: the application file is not modified, and the icon reverts when 4D quits. If the picture cannot be converted (for example an empty picture), nothing happens.

### Example

```4d
C_PICTURE($icon)
$icon:=DOCK Get icon
  // ... modify $icon with 4D picture commands ...
DOCK SET ICON ($icon)
```

---

## DOCK Get icon

### Syntax

```
DOCK Get icon → icon
```

| Parameter | Type | Description |
|---|---|---|
| Result | Picture | The current application icon, as a TIFF picture |

### Description

Returns the current application icon image (`-[NSApplication applicationIconImage]`), encoded as TIFF at the image's nominal size. If the icon cannot be rendered or encoded, the result is an empty picture. (The failure checks were added in 2.4.0; earlier builds did not check.)

### Example

```4d
C_PICTURE($icon)
$icon:=DOCK Get icon
If (Picture size($icon)>0)
	  // use $icon
End if 
```

---

## DOCK SET PROGRESS

### Syntax

```
DOCK SET PROGRESS ( progress {; options} )
```

| Parameter | Type | Description |
|---|---|---|
| `progress` | Real | Progress between `0` and `1`. `0` or less, or `1` or more, shows the plain icon |
| `options` | Object | Optional drawing options (see below) |

`options` properties:

| Property | Type | Description |
|---|---|---|
| `style` | Real | `0` = bar (default), `1` = circle, `2` = squircle (rounded square following the icon outline) |
| `lineWidth` | Real | Stroke width, or bar height, in points. Defaults: bar `10`, circle `4`, squircle `5` |
| `radius` | Real | Circle style only: radius in points. Default `30` |
| `inset` | Real | Squircle style only: extra inset in points, added to the built-in inset of `14.4` |
| `color` | Object | `red`, `green`, `blue` and optional `alpha` (default `1`), each between `0` and `1`. Default: white |

### Description

Draws a progress indicator over the application icon in the Dock. On the first call, the plugin installs an image view as the Dock tile's content view. Each call then redraws that view from the current application icon, with the indicator on top. The command returns immediately; the drawing happens on the main thread.

- When `progress` is `0` or less, or `1` or more, the plain application icon is shown. Use this to clear the indicator.
- Values in `options` must be numbers. A missing or non-numeric property uses its default. In `color`, a missing `red`, `green` or `blue` component is `0`.
- A `style` other than `0`, `1` or `2` draws the icon with no indicator.
- The Dock tile keeps the plugin's image view after the indicator is cleared, showing a snapshot of the application icon taken at the last call. (Inferred: a later [DOCK SET ICON](#dock-set-icon) may not show in the Dock until the next [DOCK SET PROGRESS](#dock-set-progress) call.)
- Builds up to release 2.3.0 do not include this command. In builds made with the macOS 14 SDK or later before 2.4.0, the circle and squircle styles could crash 4D on macOS 11 to 13.

### Example

From the plugin's test database (form `プログレス`, form method, `On Timer` branch). It cycles through the styles and colours:

```4d
	: (Form event code:C388=On Timer:K2:25)
		
		Form:C1466.progress:=Form:C1466.progress+0.01
		If (Form:C1466.progress>1)  // loop 0 to 1
			Form:C1466.progress:=Form:C1466.progress-1
			Form:C1466.options.style:=Form:C1466.options.style+1
			If (Form:C1466.options.style>2)  // loop style from 0 to 2
				Form:C1466.options.style:=0
				Form:C1466.colorIndex:=Form:C1466.colorIndex+1
				If (Form:C1466.colorIndex>=Form:C1466.colors.length)
					Form:C1466.colorIndex:=0
				End if 
				Form:C1466.options.color:=Form:C1466.colors[Form:C1466.colorIndex]
			End if 
		End if 
		
		DOCK SET PROGRESS(Form:C1466.progress; Form:C1466.options)
```

From the same form method, clearing the indicator with a single parameter (`progress` is `0` here):

```4d
			DOCK SET PROGRESS(Form:C1466.progress)
```

A red circle at 40%:

```4d
$options:=New object("style"; 1; "radius"; 40; "lineWidth"; 6; \
	"color"; New object("red"; 1; "green"; 0.2; "blue"; 0.2))
DOCK SET PROGRESS(0.4; $options)
```

---

## DOCK GET SIZE

### Syntax

```
DOCK GET SIZE ( width ; height )
```

| Parameter | Type | Description |
|---|---|---|
| `width` | Real | Receives the Dock tile width in points |
| `height` | Real | Receives the Dock tile height in points |

### Description

Returns the size of the application's Dock tile (`-[NSDockTile size]`).

### Example

```4d
C_REAL($width;$height)
DOCK GET SIZE ($width;$height)
```

---

## DOCK GET SCREEN FRAME

### Syntax

```
DOCK GET SCREEN FRAME ( x ; y ; width ; height )
```

| Parameter | Type | Description |
|---|---|---|
| `x` | Real | Receives the left edge of the visible frame |
| `y` | Real | Receives the bottom edge of the visible frame |
| `width` | Real | Receives the width of the visible frame |
| `height` | Real | Receives the height of the visible frame |

### Description

Returns the visible frame of the main screen (`[[NSScreen mainScreen] visibleFrame]`): the screen area minus the menu bar and the Dock. Values are in points, in macOS screen coordinates: the origin is the **bottom-left** corner of the primary screen and `y` grows upward, whereas 4D window coordinates start at the top-left. "Main screen" is the screen that contains the key window, which is not necessarily the primary screen. If no screen is available, all four values are `0`.

### Example

From the previous version of this README. It resizes the frontmost window so that it reaches the Dock:

```4d
DOCK GET SCREEN FRAME ($x;$y;$width;$height)

$offsetY:=102  //Design Mode Tool Bar

SET WINDOW RECT($x;$offsetY;$x+$width;$height+Menu bar height;Frontmost window)
```

---

## DOCK MENU

### Syntax

```
DOCK MENU ( options )
```

| Parameter | Type | Description |
|---|---|---|
| `options` | Object | Ignored in the current implementation |

### Description

**Experimental prototype. Do not use it in production.** The command adds a Dock menu to the 4D application, with one hard-coded item, "My method". `options` is read but not used. Choosing the item runs the project method `TEST` from a background thread outside any 4D process. That is not a supported way to run 4D code and may crash 4D. The menu cannot be removed again during the session.

---

## Error handling & troubleshooting

- **The badge does not appear.** Check the notification settings for 4D in System Settings: badges can be turned off per application.
- **The icon does not bounce.** AppKit only bounces when 4D is not the active application, so [DOCK Bounce](#dock-bounce) does nothing while 4D is frontmost.
- **[DOCK Bounce](#dock-bounce) returns 0.** The result is always `0`. Pass a variable as the parameter and read the request identifier from it after the call.
- **`Dock_Bounce_Indefinite` behaves unexpectedly.** Its value is `1`, not AppKit's critical request value `0`. Pass `0` for a bounce-until-active request.
- **[DOCK Get icon](#dock-get-icon) returns an empty picture.** The icon could not be rendered or encoded as TIFF. Check with `Picture size`.
- **The progress indicator stays visible.** Call [DOCK SET PROGRESS](#dock-set-progress) with `0` (or `1`) to show the plain icon again.
- **[DOCK SET PROGRESS](#dock-set-progress) ignores an option.** Option values must be numbers. A text value such as `"40"` is ignored and the default applies.
- **The screen frame looks upside down.** [DOCK GET SCREEN FRAME](#dock-get-screen-frame) uses bottom-left-origin macOS coordinates, not 4D's top-left window coordinates.
- **Nothing happens on Windows.** The Windows binary is a stub; the commands are macOS only.

---

## Quick reference

```4d
DOCK SET BADGE ("3")
$label:=DOCK Get badge
DOCK SET BADGE ("")

$request:=Dock_Bounce_One_Second
DOCK Bounce ($request)  // $request = request identifier
DOCK CANCEL BOUNCE ($request)

$icon:=DOCK Get icon
DOCK SET ICON ($icon)

DOCK SET PROGRESS(0.5; New object("style"; 0))
DOCK SET PROGRESS(0)

DOCK GET SIZE ($width;$height)
DOCK GET SCREEN FRAME ($x;$y;$width;$height)
```
