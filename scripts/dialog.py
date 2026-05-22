"""Shared dialog-building utilities for generating dialog JSON consumable by the engine."""

from __future__ import annotations

import json
from dataclasses import dataclass, field
from typing import Optional


# --- Named constants ---

# displayStyle (mDisplayStyle) - where/how to display
DISPLAY_FULLSCREEN = 0
DISPLAY_ACTION_AREA = 2
DISPLAY_LOWER_AREA = 3
DISPLAY_LARGE_ACTION_AREA = 5

# displayStyle2 (mDisplayStyle2) - visual style flags
STYLE_VERT_CENTERED = 0x10
STYLE_LARGE_AREA = 0x18

# displayStyle3 (mDisplayStyle3) - choice style
CHOICE_NONE = 0x00
CHOICE_RANDOM = 0x08

# Actor
ACTOR_NONE = 0
ACTOR_PARTY_LEADER = 0xFF

# Text variables
ACTIVE_PARTY_MEMBER = 11
SHOPKEEPER = 28

# --- Data classes ---


@dataclass
class DialogChoice:
    state: int = 0
    min: int = 0
    max: int = 0
    target_key: Optional[int] = None

    def to_dict(self) -> dict:
        d: dict = {"state": self.state, "min": self.min, "max": self.max}
        if self.target_key is not None:
            d["target"] = {"key": self.target_key}
        return d


@dataclass
class DialogAction:
    type: str = "SetTextVariable"
    which: int = 0
    what: int = 0

    def to_dict(self) -> dict:
        return {"type": self.type, "which": self.which, "what": self.what}


@dataclass
class DialogSnippet:
    key: int
    display_style: int = DISPLAY_FULLSCREEN
    actor: int = ACTOR_NONE
    display_style2: int = 0
    display_style3: int = 0
    text: str = ""
    choices: Optional[list[DialogChoice]] = None
    actions: Optional[list[DialogAction]] = None

    def to_dict(self) -> dict:
        snippet = {
            "displayStyle": self.display_style,
            "actor": self.actor,
            "displayStyle2": self.display_style2,
            "displayStyle3": self.display_style3,
            "text": self.text,
        }
        if self.choices:
            snippet["choices"] = [c.to_dict() for c in self.choices]
        if self.actions:
            snippet["actions"] = [a.to_dict() for a in self.actions]
        return {"key": self.key, "snippet": snippet}


@dataclass
class DialogFile:
    snippets: list[DialogSnippet] = field(default_factory=list)

    def to_dict(self) -> dict:
        return {"dialogs": [s.to_dict() for s in self.snippets]}

    def save(self, path: str) -> None:
        with open(path, "w") as f:
            json.dump(self.to_dict(), f, indent=2, ensure_ascii=False)

    @classmethod
    def load(cls, path: str) -> DialogFile:
        with open(path) as f:
            data = json.load(f)
        result = cls()
        for entry in data.get("dialogs", []):
            s = entry["snippet"]
            snippet = DialogSnippet(
                key=entry["key"],
                display_style=s.get("displayStyle", 0),
                actor=s.get("actor", 0),
                display_style2=s.get("displayStyle2", 0),
                display_style3=s.get("displayStyle3", 0),
                text=s.get("text", ""),
            )
            if "choices" in s:
                snippet.choices = [
                    DialogChoice(
                        state=c.get("state", 0),
                        min=c.get("min", 0),
                        max=c.get("max", 0),
                        target_key=c.get("target", {}).get("key"),
                    )
                    for c in s["choices"]
                ]
            if "actions" in s:
                snippet.actions = [
                    DialogAction(
                        type=a.get("type", ""),
                        which=a.get("which", 0),
                        what=a.get("what", 0),
                    )
                    for a in s["actions"]
                ]
            result.snippets.append(snippet)
        return result


# --- Builder helpers ---


def make_root_snippet(
    key: int,
    text_count: int,
    text_base: int,
) -> DialogSnippet:
    choices = [
        DialogChoice(target_key=text_base + i)
        for i in range(text_count)
    ]
    return DialogSnippet(
        key=key,
        display_style=DISPLAY_FULLSCREEN,
        display_style2=STYLE_VERT_CENTERED,
        display_style3=CHOICE_RANDOM,
        actions=[DialogAction(which=0, what=SHOPKEEPER),
                 DialogAction(which=1, what=ACTIVE_PARTY_MEMBER)],
        choices=choices,
    )


def make_text_snippet(key: int, text: str) -> DialogSnippet:
    return DialogSnippet(
        key=key,
        display_style=DISPLAY_LARGE_ACTION_AREA,
        display_style2=STYLE_LARGE_AREA,
        display_style3=CHOICE_NONE,
        text="\t" + text,
    )


def build_dialog_group(
    texts: list[str],
    root_key: int,
    text_base: int,
    text_stride: int = 1,
) -> list[DialogSnippet]:
    snippets = [make_root_snippet(root_key, len(texts), text_base)]
    for i, text in enumerate(texts):
        snippets.append(make_text_snippet(text_base + i * text_stride, text))
    return snippets
