#!/usr/bin/env python3
"""Create FYP feeds and dashboard on Adafruit IO using Secrets.h (X-AIO-Key)."""

from __future__ import annotations

import json
import re
import ssl
import sys
import urllib.error
import urllib.request
from pathlib import Path

SECRETS = Path(__file__).resolve().parent / "GeothermalMonitor" / "Secrets.h"
API = "https://io.adafruit.com/api/v2"

FEEDS = [
    ("Room temperature", "room-temp"),
    ("Outdoor temperature", "outdoor-temp"),
    ("Inlet water temperature", "inlet-temp"),
    ("Outlet water temperature", "outlet-temp"),
    ("Pipe wall temperature", "pipe-wall-temp"),
    ("Ground temperature", "ground-temp"),
]

DASHBOARD_NAME = "Geothermal FYP"
DASHBOARD_KEY = "geothermal-fyp"


def parse_secrets(path: Path) -> tuple[str, str]:
    text = path.read_text(encoding="utf-8")
    user = re.search(r'#define\s+IO_USERNAME\s+"([^"]*)"', text)
    key = re.search(r'#define\s+IO_KEY\s+"([^"]*)"', text)
    if not user or not key:
        raise SystemExit("Could not read IO_USERNAME / IO_KEY from Secrets.h")
    username = user.group(1).strip()
    aio_key = key.group(1).strip()
    if (
        not username
        or not aio_key
        or username.startswith("YOUR_")
        or aio_key.startswith("YOUR_")
    ):
        raise SystemExit(
            "Fill IO_USERNAME and IO_KEY in firmware/GeothermalMonitor/Secrets.h first."
        )
    return username, aio_key


def request(
    method: str,
    url: str,
    key: str,
    body: dict | None = None,
) -> tuple[int, object]:
    data = None if body is None else json.dumps(body).encode("utf-8")
    req = urllib.request.Request(
        url,
        data=data,
        method=method,
        headers={
            "X-AIO-Key": key,
            "Content-Type": "application/json",
            "Accept": "application/json",
        },
    )
    ctx = ssl.create_default_context()
    try:
        with urllib.request.urlopen(req, context=ctx, timeout=30) as resp:
            raw = resp.read().decode("utf-8")
            return resp.status, json.loads(raw) if raw else {}
    except urllib.error.HTTPError as exc:
        raw = exc.read().decode("utf-8", errors="replace")
        try:
            parsed = json.loads(raw)
        except json.JSONDecodeError:
            parsed = raw
        return exc.code, parsed


def ensure_feed(username: str, key: str, name: str, feed_key: str) -> None:
    code, payload = request("GET", f"{API}/{username}/feeds/{feed_key}", key)
    if code == 200:
        print(f"feed exists: {feed_key}")
        return
    code, payload = request(
        "POST",
        f"{API}/{username}/feeds",
        key,
        {"feed": {"name": name, "key": feed_key, "description": name}},
    )
    if code in (200, 201):
        print(f"feed created: {feed_key}")
        return
    raise SystemExit(f"Failed to create feed {feed_key}: {code} {payload}")


def ensure_dashboard(username: str, key: str) -> str:
    code, payload = request("GET", f"{API}/{username}/dashboards/{DASHBOARD_KEY}", key)
    if code == 200:
        print(f"dashboard exists: {DASHBOARD_KEY}")
        return DASHBOARD_KEY
    code, payload = request(
        "POST",
        f"{API}/{username}/dashboards",
        key,
        {
            "dashboard": {
                "name": DASHBOARD_NAME,
                "key": DASHBOARD_KEY,
                "description": "ESP32 DS18B20 shallow geothermal monitor",
            }
        },
    )
    if code in (200, 201) and isinstance(payload, dict):
        dash_key = payload.get("key") or DASHBOARD_KEY
        print(f"dashboard created: {dash_key}")
        return dash_key
    raise SystemExit(f"Failed to create dashboard: {code} {payload}")


def existing_block_types(username: str, key: str, dash_key: str) -> set[str]:
    code, payload = request(
        "GET", f"{API}/{username}/dashboards/{dash_key}/blocks", key
    )
    names: set[str] = set()
    if code == 200 and isinstance(payload, list):
        for block in payload:
            names.add(str(block.get("name", "")))
    return names


def create_block(
    username: str,
    key: str,
    dash_key: str,
    block: dict,
) -> dict:
    code, payload = request(
        "POST",
        f"{API}/{username}/dashboards/{dash_key}/blocks",
        key,
        {"block": block},
    )
    if code not in (200, 201) or not isinstance(payload, dict):
        raise SystemExit(f"Failed to create block {block.get('name')}: {code} {payload}")
    print(f"block created: {block.get('name')}")
    return payload


def main() -> int:
    username, aio_key = parse_secrets(SECRETS)
    code, user_data = request("GET", f"{API}/user", aio_key)
    if code != 200:
        raise SystemExit(f"Adafruit IO login failed ({code}). Check username and AIO key.")
    actual_user = user_data.get("user", {}).get("username", username) if isinstance(user_data, dict) else username
    print(f"connected as {actual_user}")

    for name, feed_key in FEEDS:
        ensure_feed(username, aio_key, name, feed_key)

    dash_key = ensure_dashboard(username, aio_key)
    already = existing_block_types(username, aio_key, dash_key)

    for index, (name, feed_key) in enumerate(FEEDS):
        gauge_name = f"{name} gauge"
        if gauge_name in already:
            print(f"block exists: {gauge_name}")
            continue
        create_block(
            username,
            aio_key,
            dash_key,
            {
                "name": gauge_name,
                "visual_type": "gauge",
                "column": index * 2,
                "row": 0,
                "size_x": 2,
                "size_y": 4,
                "block_feeds": [{"feed_id": feed_key}],
                "properties": {
                    "minValue": "0",
                    "maxValue": "50",
                    "label": "C",
                    "ringWidth": "25",
                },
            },
        )

    chart_name = "All temperatures 24h"
    if chart_name not in already:
        create_block(
            username,
            aio_key,
            dash_key,
            {
                "name": chart_name,
                "visual_type": "line_chart",
                "column": 0,
                "row": 4,
                "size_x": 12,
                "size_y": 6,
                "block_feeds": [{"feed_id": key} for _, key in FEEDS],
                "properties": {
                    "xAxisLabel": "Time",
                    "yAxisLabel": "C",
                    "yAxisMin": "0",
                    "yAxisMax": "50",
                    "historyHours": "24",
                },
            },
        )
    else:
        print(f"block exists: {chart_name}")

    print(f"Dashboard: https://io.adafruit.com/{username}/dashboards/{dash_key}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
