"""Rebuild only proven native outputs; never discard packaged siblings."""

import argparse
from pathlib import Path
import re
import shutil
import stat


PRODUCT = "Dock Tile"
BASE = Path("test/Plugins")
BUNDLE = BASE / (PRODUCT + ".bundle")
FILES = tuple(BUNDLE / relative for relative in (
    f"Contents/MacOS/{PRODUCT}", "Contents/Info.plist",
    "Contents/Resources/English.lproj/InfoPlist.strings",
    "Contents/Resources/constants.xlf", "Contents/Resources/manifest.json",
    "Contents/_CodeSignature/CodeResources",
))
PACKAGED_FILES = (*FILES, BUNDLE / f"Contents/Windows64/{PRODUCT}.4DX",
                  BUNDLE / "Contents/QuickLook/Thumbnail.png")
XCODE_DATA = Path("build/XCBuildData")
DIRECTORIES = (
    Path("build") / (PRODUCT + ".build"),
    Path("build/Default") / (PRODUCT + ".build"),
    BASE / (PRODUCT + ".build"),
    BASE / "Default" / (PRODUCT + ".build"),
)


def generated(path):
    return (path in set(map(str, FILES))
            or bool(re.fullmatch(
                r"build/XCBuildData/(?:build\.db(?:-wal|-shm)?|"
                r"[0-9a-f]{32}\.xcbuilddata/.+)", path))
            or any(path.startswith(str(directory) + "/")
                   for directory in DIRECTORIES))


def validate_paths(paths):
    root = Path.cwd().resolve()
    for path in paths:
        for parent in (path, *path.parents):
            if parent == Path("."):
                break
            if parent.is_symlink():
                raise ValueError(f"Refusing symlink output: {parent}")
        if not path.resolve().is_relative_to(root):
            raise ValueError(f"Output escapes checkout: {path}")
        if path.is_dir():
            for descendant in path.rglob("*"):
                if descendant.is_symlink():
                    raise ValueError(f"Refusing symlink output: {descendant}")
                if not (descendant.is_dir() or stat.S_ISREG(descendant.lstat().st_mode)):
                    raise ValueError(f"Refusing nonregular output: {descendant}")
        elif path.exists() and not stat.S_ISREG(path.lstat().st_mode):
            raise ValueError(f"Refusing nonregular output: {path}")


def prepare():
    files = FILES
    directories = DIRECTORIES
    validate_paths((*files, *directories, XCODE_DATA))
    for path in files:
        if path.exists() and not path.is_file():
            raise ValueError(f"Expected regular generated leaf before cleanup: {path}")
    for path in directories:
        if path.exists() and not path.is_dir():
            raise ValueError(f"Expected generated intermediate directory: {path}")
    for path in files:
        path.unlink(missing_ok=True)
    for path in directories:
        if path.exists():
            shutil.rmtree(path)


def require_file(path):
    validate_paths((path,))
    if not path.is_file() or path.stat().st_size == 0:
        raise ValueError(f"Missing or empty native output: {path}")


def verify_resources():
    for name in ("manifest.json", "constants.xlf"):
        copied = BUNDLE / "Contents/Resources" / name
        require_file(copied)
        if copied.read_bytes() != Path(name).read_bytes():
            raise ValueError(f"Copied resource differs from source: {copied}")


def validate_outputs(*, signed):
    for path in PACKAGED_FILES:
        if not signed and path == FILES[-1]:
            continue
        require_file(path)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("phase", choices=("clean-macos", "verify-resources"))
    phase = parser.parse_args().phase
    if phase == "clean-macos":
        prepare()
    else:
        verify_resources()
