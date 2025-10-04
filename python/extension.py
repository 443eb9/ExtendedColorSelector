import ctypes
import os
import sys
from krita import Extension, Krita
from pathlib import Path

if sys.platform in ["win32", "cygwin", "msys"]:
    platform = "windows"
elif sys.platform == "linux":
    platform = "linux"
elif sys.platform == "darwin":
    platform = "macos"
else:
    raise RuntimeError(f"Unsupported platform: {sys.platform}")


def _env_add_path(var: str, *paths: str | Path):
    prev = os.environ.get(var, "")
    if not paths:
        return prev
    paths = os.pathsep.join(str(p) for p in paths)
    os.environ[var] = f"{paths}{os.pathsep}{prev}" if prev else paths
    return prev


def _restore_env(var: str, value: str):
    if value:
        os.environ[var] = value


class ExtendedColorSelector(Extension):
    def __init__(self, parent):
        super().__init__(parent)

        ext = {"windows": ".dll", "linux": ".so", "macos": ".dylib"}[platform]
        lib_dir = Path(__file__).parent / "lib"
        lib_file = lib_dir / f"extendedcolorselector{ext}"

        executable_dir = Path(sys.executable).parent
        bin_paths = []
        if platform == "windows":
            bin_paths = [lib_dir, executable_dir]
        env_path = _env_add_path("PATH", *bin_paths)

        ld_paths = []
        if platform == "linux":
            ld_paths.append(lib_dir.resolve())
        if platform == "linux" and "APPDIR" in os.environ:  # for AppImage
            ld_paths.append(Path(os.environ["APPDIR"]) / "usr" / "lib")
        ld_path = _env_add_path("LD_LIBRARY_PATH", *ld_paths)

        try:
            lib = ctypes.CDLL(str(lib_file.resolve()))
            print("Trying to wake up C++ library of Extended Color Selector from Python...")
            lib.load_extended_color_selector_plugin()

        except OSError as e:
            deps = ""
            for dependency in lib_dir.glob(f"*.{ext}"):
                try:
                    ctypes.CDLL(str(dependency.resolve()))
                except OSError:
                    deps += f"\nFailed to load dependency {dependency}"
            raise RuntimeError(
                f"Failed to load ExtendedColorSelector library from {lib_file}: {e}{deps}"
            )
        finally:
            _restore_env("PATH", env_path)
            _restore_env("LD_LIBRARY_PATH", ld_path)

    def setup(self):
        pass

    def shutdown(self):
        pass

    def createActions(self, window):
        pass


Krita.instance().addExtension(ExtendedColorSelector(Krita.instance()))