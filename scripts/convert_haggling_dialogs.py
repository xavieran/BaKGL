#!/usr/bin/env python3
"""
Reads haggling_dialogs_source.json (discount ranges with text snippets)
and writes haggling_dialogs.json (generic flat dialog format).

Usage:
    python3 scripts/convert_haggling_dialogs.py
    python3 scripts/convert_haggling_dialogs.py --output path/to/output.json
"""

import argparse
import json
from dialog import DialogFile, build_dialog_group

RANGE_ROOT_BASE = 0x1B8000
TEXT_BASE = 0x1B9000
FAIL_ROOT = 0x1BA000
FAIL_TEXT_BASE = 0x1BB000
SWORD_FAIL_ROOT = 0x1BA001
SWORD_FAIL_TEXT_BASE = 0x1BC000


def convert(source_path: str, output_path: str) -> None:
    with open(source_path) as f:
        source = json.load(f)

    dialog_file = DialogFile()

    for idx, rng in enumerate(source.get("ranges", [])):
        root_key = RANGE_ROOT_BASE + idx
        text_base = TEXT_BASE + idx * 100
        dialog_file.snippets.extend(
            build_dialog_group(rng["snippets"], root_key, text_base)
        )

    fails = source.get("fails", [])
    if fails:
        dialog_file.snippets.extend(
            build_dialog_group(fails, FAIL_ROOT, FAIL_TEXT_BASE)
        )

    fails_sword = source.get("fails_sword", [])
    if fails_sword:
        dialog_file.snippets.extend(
            build_dialog_group(fails_sword, SWORD_FAIL_ROOT, SWORD_FAIL_TEXT_BASE)
        )

    dialog_file.save(output_path)
    print(f"Wrote {len(dialog_file.snippets)} dialogs to {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description="Convert haggling dialog source to generic dialog format"
    )
    parser.add_argument(
        "--source",
        help="Path to source JSON (default: data/haggling_dialogs_source.json)",
    )
    parser.add_argument(
        "--output",
        help="Path to output JSON (default: data/haggling_dialogs.json)",
    )
    args = parser.parse_args()
    convert(args.source, args.output)


if __name__ == "__main__":
    main()
