$krita = "C:\Program Files (x86)\Steam\steamapps\common\Krita\krita\bin\krita.com"

while (1) {
    Remove-Item -Path "$env:APPDATA\krita\pykrita\extended_color_selector.desktop"
    Remove-Item -Path "$env:APPDATA\krita\pykrita\extended_color_selector" -Recurse
    Copy-Item -Path ".\extended_color_selector.desktop" -Destination "$env:APPDATA\krita\pykrita"
    Copy-Item -Path ".\extended_color_selector" -Destination "$env:APPDATA\krita\pykrita" -Recurse

    & $krita
}
