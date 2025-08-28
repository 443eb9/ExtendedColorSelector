& cargo build --release
& cross build --target x86_64-unknown-linux-gnu --release

Copy-Item .\target\release\axes_limits_compute.exe .\artifacts
Copy-Item .\target\x86_64-unknown-linux-gnu\release\axes_limits_compute .\artifacts

Compress-Archive -Path .\extended_color_selector, .\extended_color_selector.desktop -DestinationPath .\artifacts\ExtendedColorSelector.zip 
