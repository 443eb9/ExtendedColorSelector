Remove-Item -Path "$env:APPDATA\krita\pykrita\extended_color_selector.desktop"
Remove-Item -Path "$env:APPDATA\krita\pykrita\extended_color_selector" -Recurse
Copy-Item -Path ".\extended_color_selector.desktop" -Destination "$env:APPDATA\krita\pykrita"
Copy-Item -Path ".\extended_color_selector" -Destination "$env:APPDATA\krita\pykrita" -Recurse
