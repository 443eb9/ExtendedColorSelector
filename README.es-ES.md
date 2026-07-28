# Extended Color Selector

Un potente selector de colores para [`Krita`](https://krita.org/) 5.

## Características

- Soporta la selección de colores en 11 modelos de color. (`Gray`, `SRGB`, `HSV`, `HSL`, `LinearRGB`, `XYZ`, `Lab`, `OkLab`, `OkLch`, `OkHsv`, `OkHsl`, `Normal`(Normal map))
- Soporte para selección de colores HDR.
- Integrado con la gestión de color de Krita. Muestra el color correcto bajo un perfil de color específico.
- Totalmente modular y altamente personalizable.
  - Tres formas de rueda: cuadrado, triángulo y círculo.
  - Deslizadores de canal y anillo para modificar el tercer canal.
  - Ejes intercambiables y reversibles.
  - El selector de color rota con el anillo.
  - Escalado para ajustarse a la gama SRGB para modelos de color en el espacio de color CIE.
- Selector de color portátil. Abre el selector en cualquier parte del lienzo usando un atajo.
- Modificadores `Shift` y `Alt` para desplazarse lentamente.
- Indicación de colores fuera de gama (out of gamut).

## Problemas conocidos

- Las capas/imágenes en espacio de color `YCbCr` provocan cierres inesperados (crash).
- Problemas de rendimiento al alterar el canal primario si se usa HDR; provoca un lag muy, muy, muuuuuuuy fuerte.
- Problemas de alias en la imagen del selector visual.
- El recorte a la gama SRGB se ve extraño bajo el espacio de color en escala de grises.

## Cómo usarlo

Ve a Releases y descarga el paquete más reciente.

Dirígete al directorio de preferencias de tu Krita, que por defecto es:

- `$HOME/.local/share/krita/` para Linux
- `%APPDATA%\krita\` para Windows
- `~/Library/Application Support/Krita/` para macOS

Descomprime el paquete del plugin en la carpeta `pykrita`. De modo que haya un archivo `extended_color_selector.desktop` y una carpeta `extended_color_selector` dentro de `pykrita`.

Inicia Krita.

Ve a `Ajustes -> Configurar Krita -> Gestor de complementos de Python`, busca `Extended Color Selector`, actívalo y reinicia Krita.

## Por qué el nombre

Para que se vea/suene como un docker nativo de Krita, al igual que el selector de colores de gama amplia y el selector de colores avanzado.

## Cómo compilar

Primero, debes seguir la página [Building Krita from Source](https://docs.krita.org/en/untranslatable_pages/building_krita.html) del manual de Krita y compilar Krita correctamente.

Luego, clona este repositorio en la carpeta `krita/plugins/`.

La estructura de directorios del código fuente de Krita debería ser así:

```
krita/
  ├── plugins/
  │    ├── ExtendedColorSelector/   # Este repositorio
  │    ├── CMakeLists.txt   # El CMakeLists.txt que vas a modificar
  │    └── ...
  └── ...
```

A continuación, abre `CMakeLists.txt` y añade esta línea después de la última línea `add_subdirectory( ... )`:

```cmake
add_subdirectory( ExtendedColorSelector )
```

Finalmente, compila de nuevo y, en el directorio de instalación, encontrarás el plugin `extended_color_selector`.

- En Windows, debería estar en `_install\extended_color_selector`
- En Linux, debería estar en `/home/appimage/appimage-workspace/krita.appdir/usr/extended_color_selector` dentro del contenedor.

Se recomienda crear enlaces simbólicos (symlinks) en la carpeta `pykrita` hacia la carpeta de instalación para no tener que copiar manualmente después de cada compilación.

## Capturas de pantalla

![](./imgs/showcase_01.png)
![](./imgs/showcase_02.png)
![](./imgs/showcase_03.png)

## Créditos

El código de `extension.py` fue copiado de [krita-vision-tools](https://github.com/Acly/krita-vision-tools), que es un excelente plugin que permite seleccionar objetos con el poder de las redes neuronales, y fue modificado ligeramente.

La mayoría de las funciones de conversión de modelos de color fueron traducidas del proyecto [`Bevy`](https://bevy.org/), específicamente del crate de rust `bevy_color`, bajo licencia `MIT`. Es un motor de juegos de código abierto muy bueno escrito en rust.

`ok_color.h` fue descargado del [Blog de Björn Ottosson](https://bottosson.github.io/misc/ok_color.h) y ajustado un poco para el proyecto, bajo licencia `MIT`.
