# Extended Color Selector

A powerful color selector for [`Krita`](https://krita.org/) 5.

## Features

- Supports color picking in 10 color models. (`Gray`, `RGB`, `HSV`, `HSL`, `XYZ`, `Lab`, `OkLab`, `OkLch`, `OkHsv`, `OkHsl`)
- Fully modular, and highly customizable.
  - Three wheel shapes: square, triangle, and circle.
  - Channel sliders and ring to modify the third channel.
  - Swappable and revertable axes.
  - Color picker rotate with ring.
  - Scale to fit SRGB gamut for color models in CIE color space.
- Flexible and tiny. Thanks to OpenGL shaders and mathemagics, we can draw color wheels realtime.
- Portable color selector. Open the selector at anywhere on canvas using shortcut.
- `Shift` and `Alt` modifiers to shift slowly.
- Out of gamut hinting.
- Integrated with color management from Krita. Display the correct color under specific color profile.

## Known Issues

Extremely lag when the dock is too large.

## How to use

Go to Releases, download the latest package.

Go to you Krita preference directory, which is, by default,

- `$HOME/.local/share/krita/` for Linux
- `%APPDATA%\krita\` for Windows
- `~/Library/Application Support/Krita/` for macOS

Unzip the plugin package into `pykrita` folder. So there will be one more `extended_color_selector.desktop` and `extended_color_selector` inside `pykrita`.

Launch Krita.

Go to `Settings -> Configure Krita -> Python Plugin Manager`, find `Extended Color Selector`, enable it, and restart Krita.

## Why the name

To make it looks/sounds like a native Krita docker, just like wide-gamut color selector and advanced color selector.

## Screenshots

// TODO

## Credits

Most color model conversion functions are translated from [`Bevy`](https://bevy.org/) project, in `bevy_color` rust crate, and under `MIT` license. It's a very nice open-source game engine written in rust.

`ok_color.h` downloaded from [Björn Ottosson's Blog](https://bottosson.github.io/misc/ok_color.h) , and tuned a bit for the project, under `MIT` license.
