#!/usr/bin/env python3
"""Build and inspect the isolated desktop category-navigation proposal."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--script", type=Path)
    parser.add_argument("--headless", action="store_true")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    subprocess.run(["make", "-f", "Makefile", "-f", "tools/discovery_preview.mk",
                    "discovery-preview"], cwd=root, check=True)
    with tempfile.TemporaryDirectory(prefix="kestrel-discovery-") as directory:
        fixture = Path(directory) / "sdcard"
        for name in ["eff", "pre", "seq"]:
            (fixture / name).mkdir(parents=True)
        # Review the authored discovery library, excluding obsolete examples.
        library = root.parent / "effects"
        for folder in [library, library / "experimental"]:
            for effect in folder.glob("*.EFF"):
                if "types:" in effect.read_text():
                    shutil.copy2(effect, fixture / "eff" / effect.name)
        command = [sys.executable, str(root / "tools/desktop_ui.py"),
                   "--binary", str(root / "bin/discovery-preview"),
                   "--sdcard", str(fixture), "--output", str(args.output.resolve())]
        if args.headless:
            command.append("--headless")
        command += ["--script", str((args.script or
                    root / "tools/ui_scripts/discovery_preview.txt").resolve())]
        subprocess.run(command, check=True)


if __name__ == "__main__":
    main()
