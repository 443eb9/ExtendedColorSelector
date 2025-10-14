# Extended Color Selector

A powerful color selector for [`Krita`](https://krita.org/) 5.

## Features

- Supports color picking in 10 color models. (`Gray`, `SRGB`, `HSV`, `HSL`, `LinearRGB`, `XYZ`, `Lab`, `OkLab`, `OkLch`, `OkHsv`, `OkHsl`)
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

## Known issues

- `YCbCr` color space layers/images causes crash.
- Performance issue when altering primary channel, really lag.
- Alias issue on visual selector image.
- Clipping to SRGB gamut looks weird under Grayscale color space.

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

## How to build

First, you need to follow [Building Krita from Source](https://docs.krita.org/en/untranslatable_pages/building_krita.html) page from Krita manual, and build Krita successfully.

Then, clone this repository into `krita/plugins/` folder.

The directory structure should be like this:

```
krita/
  ├── ...
  ├── pics/
  ├── plugins/
  │    ├── ExtendedColorSelector/   # This repository
  │    └── CMakeLists.txt   # The CMakeLists.txt you are going to modify
  ├── po/
  └── ...
```

Next, open `CMakeLists.txt`, and add the following line after the last `add_subdirectory( ... )` line:

```cmake
add_subdirectory( ExtendedColorSelector )
```

Finally, build again, and in the install directory, you will find the plugin `extended_color_selector`.

- On Windows, it should at `_install\extended_color_selector`
- On Linux, it should at `/home/appimage/.local/share/krita/pykrita/extended_color_selector`

It is recommended to create symlinks in `pykrita` folder to the install folder so you don't need to manually copy after every build.

## Screenshots

![](./imgs/showcase_01.png)
![](./imgs/showcase_02.png)
![](./imgs/showcase_03.png)

## Credits

Code from `extension.py` was copied from [krita-vision-tools](https://github.com/Acly/krita-vision-tools), which is a nice plugin allowing you to select objects with power of neural networks, and modified a little bit.

Most color model conversion functions are translated from [`Bevy`](https://bevy.org/) project, in `bevy_color` rust crate, and under `MIT` license. It's a very nice open-source game engine written in rust.

`ok_color.h` downloaded from [Björn Ottosson's Blog](https://bottosson.github.io/misc/ok_color.h) , and tuned a bit for the project, under `MIT` license.
