#!/usr/bin/env python3
"""
ContextCoreLite multi-version packager for Fab/Marketplace.

Generates one zip per UE version. Only difference between zips is
the "EngineVersion" field in .uplugin.

Usage:
    python Scripts/package.py                  # Package all versions
    python Scripts/package.py 5.4 5.5          # Package specific versions
    python Scripts/package.py --output ./dist   # Custom output dir
"""

import json
import os
import shutil
import sys
import tempfile
import zipfile
from pathlib import Path

# === Configuration ===
TARGET_VERSIONS = ["4.27", "5.0", "5.1", "5.2", "5.3", "5.4", "5.5"]

PLUGIN_NAME = "ContextCoreLite"

# Files to include in the zip (relative to plugin root)
INCLUDE_FILES = [
    "ContextCoreLite.uplugin",
    "README.md",
    "README_CN.md",
    "Source/ContextCoreLite/ContextCoreLite.Build.cs",
    "Source/ContextCoreLite/Private/BlueprintExporterLite.cpp",
    "Source/ContextCoreLite/Private/ContextCoreLiteModule.cpp",
    "Source/ContextCoreLite/Private/MetadataGeneratorLite.cpp",
    "Source/ContextCoreLite/Public/BlueprintExporterLite.h",
    "Source/ContextCoreLite/Public/ContextCoreLiteModule.h",
    "Source/ContextCoreLite/Public/MetadataGeneratorLite.h",
]

# Directories to exclude
EXCLUDE_DIRS = {"Binaries", "Intermediate", "Releases", ".git", "Scripts", "__pycache__"}
EXCLUDE_EXTS = {".swp", ".pyc", ".pdb", ".lib", ".dll", ".so", ".dylib"}


def get_plugin_root() -> Path:
    """Find plugin root (parent of Scripts/)."""
    script_dir = Path(__file__).resolve().parent
    return script_dir.parent


def patch_uplugin(uplugin_path: Path, engine_version: str) -> str:
    """Read .uplugin, set EngineVersion, return patched JSON string."""
    with open(uplugin_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    data["EngineVersion"] = engine_version

    return json.dumps(data, indent=4, ensure_ascii=False)


def create_zip(plugin_root: Path, engine_version: str, output_dir: Path) -> Path:
    """Create a zip for one UE version."""
    zip_name = f"{PLUGIN_NAME}_{engine_version}.zip"
    zip_path = output_dir / zip_name

    patched_uplugin = patch_uplugin(plugin_root / "ContextCoreLite.uplugin", engine_version)

    with zipfile.ZipFile(zip_path, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as zf:
        for rel_file in INCLUDE_FILES:
            src = plugin_root / rel_file
            if not src.exists():
                print(f"  ⚠️  Missing: {rel_file}, skipping")
                continue

            # Archive path: ContextCoreLite/<relative_path>
            arc_name = f"{PLUGIN_NAME}/{rel_file}"

            if rel_file == "ContextCoreLite.uplugin":
                # Write patched version
                zf.writestr(arc_name, patched_uplugin)
            else:
                zf.write(src, arc_name)

    return zip_path


def main():
    plugin_root = get_plugin_root()
    print(f"Plugin root: {plugin_root}")

    # Parse args
    versions = []
    output_dir = plugin_root / "Releases"

    args = sys.argv[1:]
    i = 0
    while i < len(args):
        if args[i] == "--output" and i + 1 < len(args):
            output_dir = Path(args[i + 1])
            i += 2
        elif args[i].startswith("-"):
            print(f"Unknown flag: {args[i]}")
            sys.exit(1)
        else:
            versions.append(args[i])
            i += 1

    if not versions:
        versions = TARGET_VERSIONS

    # Create output dir
    output_dir.mkdir(parents=True, exist_ok=True)

    print(f"Output: {output_dir}")
    print(f"Versions: {', '.join(versions)}")
    print()

    # Verify source files exist
    missing = []
    for f in INCLUDE_FILES:
        if not (plugin_root / f).exists():
            missing.append(f)
    if missing:
        print("❌ Missing source files:")
        for f in missing:
            print(f"   {f}")
        sys.exit(1)

    # Package each version
    results = []
    for ver in versions:
        zip_path = create_zip(plugin_root, ver, output_dir)
        size_kb = zip_path.stat().st_size / 1024
        results.append((ver, zip_path.name, size_kb))
        print(f"  ✅ {zip_path.name} ({size_kb:.1f} KB)")

    print()
    print(f"Done! {len(results)} zip(s) in {output_dir}/")
    print()

    # Summary table
    print(f"{'Version':<10} {'File':<35} {'Size':>8}")
    print("-" * 55)
    for ver, name, size in results:
        print(f"{ver:<10} {name:<35} {size:>7.1f} KB")


if __name__ == "__main__":
    main()
