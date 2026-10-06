#!/usr/bin/env python3
"""Drive the desktop UI in an isolated SD-card fixture and collect screenshots."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--script", type=Path, help="Commands: wait MS, click X Y, touch down/move X Y, touch up, tree, screenshot NAME.bmp, quit")
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--headless", action="store_true", help="Use SDL's dummy display and software renderer")
    parser.add_argument("--timeout", type=float, default=30)
    parser.add_argument("--binary", type=Path, help="Alternate desktop build to inspect")
    parser.add_argument("--sdcard", type=Path, help="SD fixture to copy instead of the checkout fixture")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    commands = args.script.read_text() if args.script else "wait 1500\ntree\nscreenshot home.bmp\nquit\n"
    screenshots = []
    lines = []
    for line in commands.splitlines():
        if line.startswith("screenshot "):
            path = Path(line[11:])
            if not path.is_absolute():
                path = output / path
            path.parent.mkdir(parents=True, exist_ok=True)
            if path.exists():
                parser.error(f"Screenshot already exists: {path}; choose a fresh output directory")
            screenshots.append(path)
            line = f"screenshot {path}"
        lines.append(line)
    if not lines or lines[-1] != "quit":
        lines.append("quit")
    env = os.environ.copy()
    if args.headless:
        env.update(SDL_VIDEODRIVER="dummy", SDL_RENDER_DRIVER="software")
    with tempfile.TemporaryDirectory(prefix="kestrel-ui-") as fixture:
        shutil.copytree(args.sdcard or root / "sdcard", Path(fixture) / "sdcard")
        try:
            result = subprocess.run([str(args.binary.resolve() if args.binary else root / "kest"), "--control"],
                                    input="\n".join(lines) + "\n", text=True,
                                    cwd=fixture, env=env, capture_output=True,
                                    timeout=args.timeout)
        except subprocess.TimeoutExpired as exc:
            (output / "control.log").write_bytes((exc.stdout or b"") + (exc.stderr or b""))
            raise SystemExit(f"Desktop UI timed out; see {output / 'control.log'}")
    (output / "control.log").write_text(result.stdout + result.stderr)
    if result.returncode or any(not path.is_file() for path in screenshots):
        raise SystemExit(f"Desktop UI failed ({result.returncode}); see {output / 'control.log'}")
    print(f"UI log: {output / 'control.log'}")
    for path in screenshots:
        print(f"Screenshot: {path}")
        try:
            from PIL import Image
        except ImportError:
            continue
        with Image.open(path) as image:
            png = path.with_suffix(".png")
            image.save(png)
            print(f"PNG: {png}")


if __name__ == "__main__":
    main()
