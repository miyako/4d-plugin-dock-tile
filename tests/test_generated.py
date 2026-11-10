"""The final publication exemption is smaller than the bundle or build roots."""

import unittest

from helper_support import source
import native


class GeneratedOutputTests(unittest.TestCase):
    def test_exact_native_files_and_controlled_directory_descendants(self):
        expected = (
            "test/Plugins/Dock Tile.bundle/Contents/MacOS/Dock Tile",
            "test/Plugins/Dock Tile.bundle/Contents/Info.plist",
            "test/Plugins/Dock Tile.bundle/Contents/Resources/English.lproj/InfoPlist.strings",
            "test/Plugins/Dock Tile.bundle/Contents/_CodeSignature/CodeResources",
            "test/Plugins/Dock Tile.bundle/Contents/Resources/constants.xlf",
            "test/Plugins/Dock Tile.bundle/Contents/Resources/manifest.json",
            "build/XCBuildData/build.db",
            "build/XCBuildData/build.db-wal",
            "build/XCBuildData/build.db-shm",
            "build/XCBuildData/0123456789abcdef0123456789abcdef.xcbuilddata/build-request.json",
            "build/Dock Tile.build/Objects-normal/arm64/main.o",
            "build/Default/Dock Tile.build/build.db",
            "test/Plugins/Dock Tile.build/Objects/file.o",
            "test/Plugins/Default/Dock Tile.build/Objects/file.o",
        )
        for path in expected:
            with self.subTest(path=path):
                self.assertTrue(native.generated(path))

    def test_every_resource_source_or_unproven_output_stays_protected(self):
        excluded = (
            "test/Plugins/Dock Tile.bundle/Contents/Windows/Dock Tile.4DX",
            "test/Plugins/Dock Tile.bundle/Contents/Windows64/Dock Tile.4DX",
            "test/Plugins/Dock Tile.bundle/Contents/MacOS/runtime.dylib",
            "test/Plugins/Dock Tile.bundle/Contents/QuickLook/Thumbnail.png",
            "test/Plugins/Default/Dock Tile.bundle/Contents/Info.plist",
            "test/Plugins/Default/keep", "build/XCBuildData-evil/build.db",
            "build/unexpected-source.cpp",
            "build/XCBuildData/unexpected-source.cpp",
            "build/XCBuildData/build.db-evil",
            "build/XCBuildData/evil.xcbuilddata/build-request.json",
            "build/Dock Tile.build-evil/injected.cpp",
            "test/Plugins/Dock Tile.bundle/Contents/_CodeSignature-evil/file",
            "test/Plugins/Dock Tile.bundle/Contents/Info.plist.backup",
            *map(str, source.SOURCE_PATHS),
        )
        for path in excluded:
            with self.subTest(path=path):
                self.assertFalse(native.generated(path))
