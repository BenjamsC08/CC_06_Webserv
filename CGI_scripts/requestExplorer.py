#!/usr/bin/env python3

import os
import sys
import html
from urllib.parse import parse_qs


MAX_BODY_PREVIEW = 32768
MAX_ENV_VALUE_PREVIEW = 4096


def html_escape(value):
    return html.escape(str(value), quote=True)


def read_request_body():
    content_length = os.environ.get("CONTENT_LENGTH", "")
    if not content_length:
        return b""
    try:
        length = int(content_length)
    except ValueError:
        return b""
    if length <= 0:
        return b""
    return sys.stdin.buffer.read(length)


def format_header_name(env_name):
    if env_name == "CONTENT_TYPE":
        return "Content-Type"
    if env_name == "CONTENT_LENGTH":
        return "Content-Length"
    if not env_name.startswith("HTTP_"):
        return env_name
    header = env_name[5:].replace("_", "-").lower()
    return "-".join(part.capitalize() for part in header.split("-"))


def extract_request_headers(env):
    headers = {}
    for key, value in env.items():
        if key.startswith("HTTP_") or key in ("CONTENT_TYPE", "CONTENT_LENGTH"):
            headers[format_header_name(key)] = value
    return dict(sorted(headers.items(), key=lambda item: item[0].lower()))


def parse_query_string():
    raw_query = os.environ.get("QUERY_STRING", "")
    return parse_qs(raw_query, keep_blank_values=True)


def parse_urlencoded_body(body_bytes):
    content_type = os.environ.get("CONTENT_TYPE", "")
    if "application/x-www-form-urlencoded" not in content_type:
        return {}
    try:
        body_text = body_bytes.decode("utf-8", errors="replace")
    except Exception:
        return {}
    return parse_qs(body_text, keep_blank_values=True)


def bytes_to_text_preview(data, limit=MAX_BODY_PREVIEW):
    truncated = False
    if len(data) > limit:
        data = data[:limit]
        truncated = True
    text = data.decode("utf-8", errors="replace")
    return text, truncated


def hexdump(data, width=16, limit=1024):
    if len(data) > limit:
        data = data[:limit]
        truncated = True
    else:
        truncated = False

    lines = []
    for offset in range(0, len(data), width):
        chunk = data[offset:offset + width]
        hex_part = " ".join(f"{byte:02x}" for byte in chunk)
        ascii_part = "".join(chr(byte) if 32 <= byte <= 126 else "." for byte in chunk)
        lines.append(f"{offset:08x}  {hex_part:<47}  {ascii_part}")
    return "\n".join(lines), truncated


def render_key_value_table(title, mapping):
    rows = []
    for key, value in mapping.items():
        safe_key = html_escape(key)
        safe_value = html_escape(value)
        rows.append(
            f"<tr><td class=\"key-col\">{safe_key}</td>"
            f"<td><code>{safe_value}</code></td></tr>"
        )
    if not rows:
        rows.append("<tr><td colspan=\"2\"><em>(empty)</em></td></tr>")
    return (
        f"<div class=\"panel\">"
        f"<h2>{html_escape(title)}</h2>"
        f"<table>{''.join(rows)}</table>"
        f"</div>"
    )


def render_multi_value_table(title, mapping):
    rows = []
    for key, values in mapping.items():
        joined = ", ".join(html_escape(v) for v in values)
        rows.append(
            f"<tr><td class=\"key-col\">{html_escape(key)}</td>"
            f"<td><code>{joined}</code></td></tr>"
        )
    if not rows:
        rows.append("<tr><td colspan=\"2\"><em>(empty)</em></td></tr>")
    return (
        f"<div class=\"panel\">"
        f"<h2>{html_escape(title)}</h2>"
        f"<table>{''.join(rows)}</table>"
        f"</div>"
    )


def render_environment_table(env):
    rows = []
    for key in sorted(env.keys()):
        value = env[key]
        if len(value) > MAX_ENV_VALUE_PREVIEW:
            value = value[:MAX_ENV_VALUE_PREVIEW] + " ... [truncated]"
        rows.append(
            f"<tr><td class=\"key-col\">{html_escape(key)}</td>"
            f"<td><code>{html_escape(value)}</code></td></tr>"
        )
    if not rows:
        rows.append("<tr><td colspan=\"2\"><em>(empty)</em></td></tr>")
    return (
        "<div class=\"panel\">"
        "<h2>CGI Environment</h2>"
        f"<table>{''.join(rows)}</table>"
        "</div>"
    )


def build_page():
    env = dict(os.environ)
    body = read_request_body()

    method = env.get("REQUEST_METHOD", "GET")
    query_string = env.get("QUERY_STRING", "")
    content_type = env.get("CONTENT_TYPE", "")
    content_length = env.get("CONTENT_LENGTH", "")
    cwd = os.getcwd()

    headers = extract_request_headers(env)
    query_params = parse_query_string()
    form_params = parse_urlencoded_body(body)

    selected_info = {
        "Request method": method,
        "Query string": query_string if query_string else "(empty)",
        "Declared Content-Length": content_length if content_length else "(absent)",
        "Declared Content-Type": content_type if content_type else "(absent)",
        "Actual body size (bytes read from stdin)": str(len(body)),
        "Current working directory": cwd,
        "Script name": env.get("SCRIPT_NAME", "(absent)"),
        "Path info": env.get("PATH_INFO", "(absent)"),
        "Server protocol": env.get("SERVER_PROTOCOL", "(absent)"),
        "Server name": env.get("SERVER_NAME", "(absent)"),
        "Server port": env.get("SERVER_PORT", "(absent)"),
        "Remote address": env.get("REMOTE_ADDR", "(absent)"),
        "Remote port": env.get("REMOTE_PORT", "(absent)"),
        "Gateway interface": env.get("GATEWAY_INTERFACE", "(absent)"),
    }

    body_text_preview, text_truncated = bytes_to_text_preview(body)
    body_hex_preview, hex_truncated = hexdump(body)

    query_table = {key: values for key, values in query_params.items()}
    form_table = {key: values for key, values in form_params.items()}

    sections = []
    sections.append(render_key_value_table("Request Summary", selected_info))
    sections.append(render_key_value_table("Received Headers", headers))
    sections.append(render_multi_value_table("Parsed Query Parameters", query_table))
    sections.append(render_multi_value_table("Parsed Form Parameters", form_table))

    body_preview_note = []
    if text_truncated:
        body_preview_note.append("Text preview truncated.")
    if hex_truncated:
        body_preview_note.append("Hex preview truncated.")
    body_preview_note_text = " ".join(body_preview_note) if body_preview_note else "Full preview shown."

    sections.append(
        "<div class=\"panel\">"
        "<h2>Raw Body - Text Preview</h2>"
        f"<p class=\"small-note\">{html_escape(body_preview_note_text)}</p>"
        f"<pre>{html_escape(body_text_preview) if body_text_preview else html_escape('(empty)')}</pre>"
        "</div>"
    )

    sections.append(
        "<div class=\"panel\">"
        "<h2>Raw Body - Hex Preview</h2>"
        f"<pre>{html_escape(body_hex_preview) if body_hex_preview else html_escape('(empty)')}</pre>"
        "</div>"
    )

    sections.append(render_environment_table(env))

    html_page = f"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CGI Request Explorer</title>
<style>
body {{
    background: #56606f;
    color: #1f2328;
    font-family: Tahoma, Verdana, Arial, sans-serif;
    margin: 0;
    padding: 36px;
}}

.window {{
    max-width: 1180px;
    margin: 0 auto;
    background: #aeb6c2;
    border: 2px solid;
    border-color: #d7dde5 #4a5260 #4a5260 #d7dde5;
    box-shadow: 8px 8px 0 rgba(20, 24, 30, 0.35);
}}

.title-bar {{
    background: linear-gradient(to right, #313a46, #667181);
    color: #f3f6fa;
    padding: 6px 8px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    font-size: 13px;
    font-weight: bold;
    letter-spacing: 0.2px;
}}

.title-bar-buttons {{
    display: flex;
    gap: 3px;
    margin-left: 10px;
}}

.title-button {{
    width: 18px;
    height: 16px;
    background: #b8c0cb;
    border: 1px solid;
    border-color: #e1e6ed #4a5260 #4a5260 #e1e6ed;
    color: #20242a;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 10px;
    line-height: 1;
    box-sizing: border-box;
}}

.content {{
    padding: 18px;
    background: #aeb6c2;
}}

.panel {{
    background: #bcc4cf;
    border: 2px solid;
    border-color: #dce2e8 #56606f #56606f #dce2e8;
    padding: 14px;
    margin-bottom: 16px;
}}

h1 {{
    margin: 0 0 10px 0;
    font-size: 28px;
    font-weight: bold;
    color: #1c2128;
}}

h2 {{
    margin: 0 0 12px 0;
    font-size: 20px;
    font-weight: bold;
    color: #1f252c;
}}

.subtitle {{
    margin: 0 0 14px 0;
    font-size: 14px;
    line-height: 1.5;
    color: #303741;
}}

.note {{
    margin-top: 14px;
    padding: 10px 12px;
    font-size: 13px;
    line-height: 1.5;
    color: #2a3139;
    background: #c7ced7;
    border: 1px solid;
    border-color: #e1e6ed #6d7785 #6d7785 #e1e6ed;
}}

.small-note {{
    margin: 0 0 10px 0;
    color: #2f363f;
    font-size: 13px;
}}

label {{
    color: #232931;
    font-size: 14px;
}}

input[type="text"],
textarea {{
    width: 100%;
    font-family: "Courier New", monospace;
    font-size: 14px;
    line-height: 1.45;
    padding: 12px;
    box-sizing: border-box;
    background: #eef2f6;
    color: #11161b;
    border: 2px solid;
    border-color: #5d6674 #e0e5eb #e0e5eb #5d6674;
    outline: none;
}}

textarea {{
    min-height: 180px;
    resize: vertical;
}}

input[type="file"] {{
    font-size: 13px;
}}

button {{
    padding: 8px 16px;
    font-size: 13px;
    font-family: Tahoma, Verdana, Arial, sans-serif;
    font-weight: bold;
    cursor: pointer;
    background: #b7bfca;
    color: #1b2026;
    border: 2px solid;
    border-color: #e1e6ed #55606e #55606e #e1e6ed;
    min-width: 130px;
}}

button:active {{
    border-color: #55606e #e1e6ed #e1e6ed #55606e;
    padding: 9px 15px 7px 17px;
}}

.form-grid {{
    display: grid;
    grid-template-columns: 1fr;
    gap: 16px;
}}

table {{
    width: 100%;
    border-collapse: collapse;
    background: #c3cad4;
    border: 2px solid;
    border-color: #dfe4ea #596270 #596270 #dfe4ea;
}}

th, td {{
    padding: 10px 12px;
    text-align: left;
    border: 1px solid #7b8593;
    vertical-align: top;
}}

th {{
    background: #8e99a8;
    color: #11161b;
    font-weight: bold;
}}

td {{
    background: #d5dbe3;
    color: #1c2127;
}}

tr:nth-child(even) td {{
    background: #cbd2db;
}}

.key-col {{
    width: 280px;
    font-weight: bold;
}}

pre {{
    margin: 0;
    padding: 12px;
    background: #eef2f6;
    color: #14191f;
    border: 2px solid;
    border-color: #5d6674 #e0e5eb #e0e5eb #5d6674;
    white-space: pre-wrap;
    word-break: break-word;
    font-family: "Courier New", monospace;
    line-height: 1.45;
    overflow-x: auto;
}}

code {{
    font-family: "Courier New", monospace;
}}

hr {{
    border: none;
    border-top: 1px solid #7b8593;
    margin: 18px 0;
}}
</style>
</head>
<body>
<div class="window">
    <div class="title-bar">
        <div>CGI Request Explorer</div>
        <div class="title-bar-buttons">
            <div class="title-button">_</div>
            <div class="title-button">□</div>
            <div class="title-button">×</div>
        </div>
    </div>

    <div class="content">
        <div class="panel">
            <h1>CGI Request Explorer</h1>
            <p class="subtitle">
                This page helps validate the full CGI pipeline: method, query string, headers, raw body,
                CGI environment, body size, and current working directory.
            </p>
            <p class="note">
                If one of the POST forms below sends you to a 405 error page, make sure the
                <code>POST</code> method is explicitly allowed for this location in the configuration file.
            </p>
        </div>

        <div class="panel">
            <h2>Built-in Test Forms</h2>

            <div class="form-grid">
                <form method="GET" action="">
                    <label for="get_message"><strong>GET test</strong></label><br><br>
                    <input type="text" id="get_message" name="message" placeholder="Type a value for the query string">
                    <br><br>
                    <button type="submit">Send GET request</button>
                </form>

                <hr>

                <form method="POST" action="">
                    <label for="post_text"><strong>POST test - application/x-www-form-urlencoded</strong></label><br><br>
                    <textarea id="post_text" name="text" placeholder="Type text here"></textarea>
                    <br><br>
                    <button type="submit">Send POST request</button>
                </form>

                <hr>

                <form method="POST" action="" enctype="multipart/form-data">
                    <label for="upload_file"><strong>POST test - multipart/form-data upload</strong></label><br><br>
                    <input type="file" id="upload_file" name="upload_file">
                    <br><br>
                    <button type="submit">Upload file</button>
                </form>
            </div>
        </div>

        {''.join(sections)}
    </div>
</div>
</body>
</html>"""
    return html_page


def main():
    page = build_page()
    response = "Content-Type: text/html; charset=UTF-8\r\n\r\n" + page
    sys.stdout.write(response)


if __name__ == "__main__":
    main()
