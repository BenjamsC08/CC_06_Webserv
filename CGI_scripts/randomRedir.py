#!/usr/bin/env python3

import html
import secrets
import sys
from urllib.parse import parse_qs
import os


LINKS = [
    {"theme": "cryptography", "title": "NIST Cryptography", "url": "https://www.nist.gov/cryptography"},
    {"theme": "cryptography", "title": "NIST Lightweight Cryptography", "url": "https://csrc.nist.gov/projects/lightweight-cryptography"},
    {"theme": "cryptography", "title": "MDN - Cryptography", "url": "https://developer.mozilla.org/en-US/docs/Glossary/Cryptography"},
    {"theme": "cryptography", "title": "MDN - Web Crypto API", "url": "https://developer.mozilla.org/en-US/docs/Web/API/Web_Crypto_API"},
    {"theme": "cryptography", "title": "MDN - SubtleCrypto", "url": "https://developer.mozilla.org/en-US/docs/Web/API/SubtleCrypto"},
    {"theme": "cryptography", "title": "Cryptopals", "url": "https://cryptopals.com/"},
    {"theme": "cryptography", "title": "Cryptopals Set 1", "url": "https://cryptopals.com/sets/1"},
    {"theme": "cryptography", "title": "OWASP Cryptographic Storage Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/Cryptographic_Storage_Cheat_Sheet.html"},

    {"theme": "satoshi_nakamoto", "title": "Bitcoin Whitepaper", "url": "https://bitcoin.org/bitcoin.pdf"},
    {"theme": "satoshi_nakamoto", "title": "Bitcoin.org - Bitcoin Paper", "url": "https://bitcoin.org/en/bitcoin-paper"},
    {"theme": "satoshi_nakamoto", "title": "Wikipedia - Satoshi Nakamoto", "url": "https://en.wikipedia.org/wiki/Satoshi_Nakamoto"},
    {"theme": "satoshi_nakamoto", "title": "Britannica - Satoshi Nakamoto", "url": "https://www.britannica.com/biography/Satoshi-Nakamoto"},
    {"theme": "satoshi_nakamoto", "title": "Bitcoin Wiki - Satoshi Nakamoto", "url": "https://en.bitcoin.it/wiki/Satoshi_Nakamoto"},
    {"theme": "satoshi_nakamoto", "title": "The Complete Satoshi", "url": "https://satoshi.nakamotoinstitute.org/"},
    {"theme": "satoshi_nakamoto", "title": "Nakamoto Institute", "url": "https://nakamotoinstitute.org/"},

    {"theme": "bitcoin", "title": "Bitcoin.org", "url": "https://bitcoin.org/"},
    {"theme": "bitcoin", "title": "Bitcoin.org - How it works", "url": "https://bitcoin.org/en/how-it-works"},
    {"theme": "bitcoin", "title": "Bitcoin Wiki - Main Page", "url": "https://en.bitcoin.it/wiki/Main_Page"},
    {"theme": "bitcoin", "title": "Bitcoin Wiki - Bitcoin", "url": "https://en.bitcoin.it/wiki/Bitcoin"},
    {"theme": "bitcoin", "title": "Bitcoin Wiki - Genesis block", "url": "https://en.bitcoin.it/wiki/Genesis_block"},
    {"theme": "bitcoin", "title": "Bitcoin Wiki - Protocol documentation", "url": "https://en.bitcoin.it/wiki/Protocol_documentation"},
    {"theme": "bitcoin", "title": "Wikipedia - History of Bitcoin", "url": "https://en.wikipedia.org/wiki/History_of_bitcoin"},

    {"theme": "rats", "title": "Wikipedia - Rat", "url": "https://en.wikipedia.org/wiki/Rat"},
    {"theme": "rats", "title": "Wikipedia - Brown rat", "url": "https://en.wikipedia.org/wiki/Brown_rat"},
    {"theme": "rats", "title": "CDC - Rodent control", "url": "https://www.cdc.gov/healthy-pets/rodent-control/index.html"},
    {"theme": "rats", "title": "CDC - Clean up", "url": "https://www.cdc.gov/healthy-pets/rodent-control/clean-up.html"},
    {"theme": "rats", "title": "CDC - Seal up", "url": "https://www.cdc.gov/healthy-pets/rodent-control/seal-up.html"},
    {"theme": "rats", "title": "CDC - Trap up", "url": "https://www.cdc.gov/healthy-pets/rodent-control/trap-up.html"},

    {"theme": "computer_security", "title": "OWASP Cheat Sheets", "url": "https://owasp.org/www-project-cheat-sheets/"},
    {"theme": "computer_security", "title": "OWASP Cheat Sheet Series", "url": "https://cheatsheetseries.owasp.org/index.html"},
    {"theme": "computer_security", "title": "OWASP Top Ten Index", "url": "https://cheatsheetseries.owasp.org/IndexTopTen.html"},
    {"theme": "computer_security", "title": "OWASP Web Service Security Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/Web_Service_Security_Cheat_Sheet.html"},
    {"theme": "computer_security", "title": "OWASP REST Security Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/REST_Security_Cheat_Sheet.html"},
    {"theme": "computer_security", "title": "OWASP Authentication Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/Authentication_Cheat_Sheet.html"},
    {"theme": "computer_security", "title": "OWASP Password Storage Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/Password_Storage_Cheat_Sheet.html"},
    {"theme": "computer_security", "title": "OWASP Secure Code Review Cheat Sheet", "url": "https://cheatsheetseries.owasp.org/cheatsheets/Secure_Code_Review_Cheat_Sheet.html"},

    {"theme": "low_level_programming", "title": "OSDev - Bare Bones", "url": "https://wiki.osdev.org/User:Zesterer/Bare_Bones"},
    {"theme": "low_level_programming", "title": "OSDev - ELF Tutorial", "url": "https://wiki.osdev.org/ELF_Tutorial"},
    {"theme": "low_level_programming", "title": "OSDev - GDT Tutorial", "url": "https://wiki.osdev.org/GDT_Tutorial"},
    {"theme": "low_level_programming", "title": "OSDev - X86-64", "url": "https://wiki.osdev.org/X86-64"},
    {"theme": "low_level_programming", "title": "OSDev - X86-64 Instruction Encoding", "url": "https://wiki.osdev.org/X86-64_Instruction_Encoding"},
    {"theme": "low_level_programming", "title": "OSDev - GCC Cross-Compiler", "url": "https://wiki.osdev.org/GCC_Cross-Compiler"},
    {"theme": "low_level_programming", "title": "Linux kernel docs - Howto", "url": "https://docs.kernel.org/process/howto.html"},
    {"theme": "low_level_programming", "title": "Linux kernel docs - Coding style", "url": "https://docs.kernel.org/process/coding-style.html"},

    {"theme": "cgi_1_1", "title": "RFC 3875 - CGI/1.1", "url": "https://www.rfc-editor.org/rfc/rfc3875.html"},
    {"theme": "cgi_1_1", "title": "RFC 3875 info page", "url": "https://www.rfc-editor.org/info/rfc3875"},
    {"theme": "cgi_1_1", "title": "RFC 3875 errata", "url": "https://www.rfc-editor.org/errata/rfc3875"},
    {"theme": "cgi_1_1", "title": "RFC 3875 errata 5266", "url": "https://www.rfc-editor.org/errata/eid5266"},
    {"theme": "cgi_1_1", "title": "RFC 3875 errata 5880", "url": "https://www.rfc-editor.org/errata/eid5880"},

    {"theme": "rtfm", "title": "man7 - Linux man-pages", "url": "https://man7.org/linux/man-pages/"},
    {"theme": "rtfm", "title": "man7 - man(1)", "url": "https://man7.org/linux/man-pages/man1/man.1.html"},
    {"theme": "rtfm", "title": "man7 - intro(1)", "url": "https://man7.org/linux/man-pages/man1/intro.1.html"},
    {"theme": "rtfm", "title": "man7 - man-pages(7)", "url": "https://man7.org/linux/man-pages/man7/man-pages.7.html"},
    {"theme": "rtfm", "title": "man7 - alphabetical index", "url": "https://man7.org/linux/man-pages/dir_all_alphabetic.html"},
    {"theme": "rtfm", "title": "ArchWiki - General recommendations", "url": "https://wiki.archlinux.org/title/General_recommendations"},
]

VALID_THEMES = sorted(set(link["theme"] for link in LINKS))


def h(value):
    return html.escape(str(value), quote=True)


def write_headers(status, content_type="text/html; charset=UTF-8", extra_headers=None):
    sys.stdout.write("Status: " + status + "\r\n")
    sys.stdout.write("Content-Type: " + content_type + "\r\n")
    sys.stdout.write("Cache-Control: no-store, max-age=0\r\n")
    sys.stdout.write("X-Content-Type-Options: nosniff\r\n")
    if extra_headers:
        for header in extra_headers:
            sys.stdout.write(header + "\r\n")
    sys.stdout.write("\r\n")


def get_query():
    raw_query = os.environ.get("QUERY_STRING", "")
    return parse_qs(raw_query, keep_blank_values=True)


def get_selected_themes(query):
    raw_values = query.get("theme", [])
    selected = []

    for value in raw_values:
        parts = value.split(",")
        for part in parts:
            theme = part.strip().lower()
            if theme and theme not in selected:
                selected.append(theme)

    if not selected:
        return VALID_THEMES[:], None

    for theme in selected:
        if theme not in VALID_THEMES:
            return None, theme
    return selected, None


def filter_links(themes):
    return [link for link in LINKS if link["theme"] in themes]


def pick_random_link(pool):
    if not pool:
        return None
    return pool[secrets.randbelow(len(pool))]


def theme_counts():
    counts = {}
    for theme in VALID_THEMES:
        counts[theme] = 0
    for link in LINKS:
        counts[link["theme"]] += 1
    return counts


def build_theme_query(themes):
    return "&".join("theme=" + theme for theme in themes)


def render_page(title, body_html, extra_head=""):
    return """<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>%s</title>
%s
<style>
body {
	background: #56606f;
	color: #1f2328;
	font-family: Tahoma, Verdana, Arial, sans-serif;
	margin: 0;
	padding: 36px;
}

.window {
	max-width: 1080px;
	margin: 0 auto;
	background: #aeb6c2;
	border: 2px solid;
	border-color: #d7dde5 #4a5260 #4a5260 #d7dde5;
	box-shadow: 8px 8px 0 rgba(20, 24, 30, 0.35);
}

.title-bar {
	background: linear-gradient(to right, #313a46, #667181);
	color: #f3f6fa;
	padding: 6px 8px;
	display: flex;
	justify-content: space-between;
	align-items: center;
	font-size: 13px;
	font-weight: bold;
	letter-spacing: 0.2px;
}

.title-bar-buttons {
	display: flex;
	gap: 3px;
}

.title-button {
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
}

.content {
	padding: 18px;
	background: #aeb6c2;
}

.panel {
	background: #bcc4cf;
	border: 2px solid;
	border-color: #dce2e8 #56606f #56606f #dce2e8;
	padding: 14px;
	margin-bottom: 16px;
}

h1 {
	margin: 0 0 10px 0;
	font-size: 28px;
	color: #1c2128;
}

h2 {
	margin: 0 0 12px 0;
	font-size: 20px;
	color: #1f252c;
}

h3 {
	margin: 0 0 10px 0;
	font-size: 16px;
	color: #232931;
}

p, li {
	line-height: 1.5;
	color: #2f363f;
}

.note {
	margin-top: 14px;
	padding: 10px 12px;
	font-size: 13px;
	line-height: 1.5;
	color: #2a3139;
	background: #c7ced7;
	border: 1px solid;
	border-color: #e1e6ed #6d7785 #6d7785 #e1e6ed;
}

.info-box {
	margin-top: 12px;
	padding: 12px;
	background: #eef2f6;
	border: 2px solid;
	border-color: #5d6674 #e0e5eb #e0e5eb #5d6674;
	font-family: "Courier New", monospace;
	color: #14191f;
	white-space: pre-wrap;
	word-break: break-word;
}

.theme-grid {
	display: grid;
	grid-template-columns: repeat(auto-fit, minmax(230px, 1fr));
	gap: 10px;
	margin-top: 12px;
}

.theme-item {
	background: #c7ced7;
	border: 1px solid;
	border-color: #e1e6ed #6d7785 #6d7785 #e1e6ed;
	padding: 10px;
}

.theme-item label {
	display: block;
	cursor: pointer;
	font-size: 14px;
	color: #20262d;
}

.actions {
	display: flex;
	flex-wrap: wrap;
	gap: 10px;
	margin-top: 16px;
}

button,
.button-link {
	padding: 8px 16px;
	font-size: 13px;
	font-family: Tahoma, Verdana, Arial, sans-serif;
	font-weight: bold;
	cursor: pointer;
	background: #b7bfca;
	color: #1b2026;
	border: 2px solid;
	border-color: #e1e6ed #55606e #55606e #e1e6ed;
	text-decoration: none;
	display: inline-block;
	box-sizing: border-box;
}

button:active,
.button-link:active {
	border-color: #55606e #e1e6ed #e1e6ed #55606e;
	padding: 9px 15px 7px 17px;
}

a {
	color: #223b63;
}

ul {
	margin: 8px 0 0 18px;
	padding: 0;
}

small {
	color: #4a5260;
}

.counter {
	font-weight: bold;
	color: #1b2026;
}

.catalog-list {
	margin-top: 14px;
}

.catalog-list li {
	margin-bottom: 8px;
}

.inline-code {
	font-family: "Courier New", monospace;
	background: #eef2f6;
	padding: 1px 4px;
	border: 1px solid #9aa3af;
}
</style>
</head>
<body>
<div class="window">
	<div class="title-bar">
		<div>%s</div>
		<div class="title-bar-buttons">
			<div class="title-button">_</div>
			<div class="title-button">□</div>
			<div class="title-button">×</div>
		</div>
	</div>
	<div class="content">
		%s
	</div>
</div>
</body>
</html>""" % (h(title), extra_head, h(title), body_html)


def render_home(selected_themes):
    counts = theme_counts()
    theme_blocks = []

    for theme in VALID_THEMES:
        checked = " checked" if theme in selected_themes else ""
        theme_blocks.append("""
<div class="theme-item">
	<label>
		<input type="checkbox" name="theme" value="%s"%s>
		<strong>%s</strong><br>
		<small>%d link(s)</small>
	</label>
</div>""" % (h(theme), checked, h(theme), counts[theme]))

    quick_all = "?action=pick"
    quick_bitcoin = "?action=pick&theme=bitcoin&theme=satoshi_nakamoto"
    quick_security = "?action=pick&theme=cryptography&theme=computer_security&theme=rtfm"
    quick_low = "?action=pick&theme=low_level_programming&theme=cgi_1_1&theme=rtfm"
    quick_rats = "?action=pick&theme=rats"

    body = """
<div class="panel">
	<h1>Random Link Redirector</h1>
	<p>This local CGI page lets you select one or more themes, pick a random whitelisted URL, and then pass through a local landing page before the external redirect.</p>
</div>

<div class="panel">
	<h2>Select one or more themes</h2>
	<form method="GET" action="">
		<input type="hidden" name="action" value="pick">
		<div class="theme-grid">
			%s
		</div>

		<div class="actions">
			<button type="submit">Pick random link</button>
			<a class="button-link" href="?">Reset selection</a>
			<a class="button-link" href="?action=list">Show full catalog</a>
		</div>
	</form>
</div>

<div class="panel">
	<h2>Quick presets</h2>
	<div class="actions">
		<a class="button-link" href="%s">Surprise me</a>
		<a class="button-link" href="%s">Bitcoin + Satoshi</a>
		<a class="button-link" href="%s">Crypto + Security + RTFM</a>
		<a class="button-link" href="%s">Low-level + CGI + RTFM</a>
		<a class="button-link" href="%s">Rats only</a>
	</div>
</div>
""" % (
        "".join(theme_blocks),
        quick_all,
        quick_bitcoin,
        quick_security,
        quick_low,
        quick_rats,
    )

    return render_page("Random Link Redirector", body)


def render_catalog(selected_themes):
    theme_query = build_theme_query(selected_themes)
    blocks = []

    for theme in VALID_THEMES:
        theme_links = [link for link in LINKS if link["theme"] == theme]
        items = []
        for link in theme_links:
            items.append('<li><a href="%s">%s</a><br><small>%s</small></li>' % (
                h(link["url"]),
                h(link["title"]),
                h(link["url"])
            ))
        blocks.append("""
<div class="panel">
	<h2>%s</h2>
	<ul class="catalog-list">
		%s
	</ul>
</div>""" % (h(theme), "".join(items)))

    body = """
<div class="panel">
	<h1>Catalog</h1>
	<p>This page lists the full local whitelist used by the random redirector.</p>
	<div class="actions">
		<a class="button-link" href="?">Back to selector</a>
		<a class="button-link" href="?action=pick%s">Pick random link from current selection</a>
	</div>
</div>
%s
""" % (
        "&" + theme_query if theme_query else "",
        "".join(blocks)
    )

    return render_page("Catalog", body)


def render_bad_request(invalid_theme):
    body = """
<div class="panel">
	<h1>Bad Request</h1>
	<p>The theme <span class="inline-code">%s</span> is not valid.</p>
	<p>Valid themes are:</p>
	<div class="info-box">%s</div>
	<div class="actions">
		<a class="button-link" href="?">Back to selector</a>
	</div>
</div>
""" % (h(invalid_theme), h(", ".join(VALID_THEMES)))

    return render_page("Bad Request", body)


def render_no_result():
    body = """
<div class="panel">
	<h1>No Result</h1>
	<p>No link could be selected from the current theme selection.</p>
	<div class="actions">
		<a class="button-link" href="?">Back to selector</a>
	</div>
</div>
"""
    return render_page("No Result", body)


def render_redirect_landing(link, selected_themes):
    selected_theme_text = ", ".join(selected_themes)
    extra_head = """
<meta http-equiv="refresh" content="2;url=%s">
<script>
let remaining = 2;
function updateCountdown() {
	var node = document.getElementById("countdown");
	if (!node)
		return;
	node.textContent = String(remaining);
	if (remaining > 0)
		remaining -= 1;
}
window.onload = function() {
	updateCountdown();
	setInterval(updateCountdown, 1000);
	setTimeout(function() {
		window.location.href = %s;
	}, 2000);
};
</script>
""" % (h(link["url"]), repr(link["url"]))

    body = """
<div class="panel">
	<h1>Redirecting soon...</h1>
	<p>A random entry has been selected from your local theme selection.</p>
	<p class="note">You will be redirected automatically in <span id="countdown" class="counter">2</span> second(s).</p>
</div>

<div class="panel">
	<h2>Selected entry</h2>
	<div class="info-box">Theme : %s
Title : %s
URL   : %s</div>

	<div class="actions">
		<a class="button-link" href="%s">Open now</a>
		<a class="button-link" href="?action=pick&%s">Pick another from same themes</a>
		<a class="button-link" href="?">Back to selector</a>
		<a class="button-link" href="?action=list&%s">Show catalog</a>
	</div>
</div>
""" % (
        h(link["theme"]),
        h(link["title"]),
        h(link["url"]),
        h(link["url"]),
        build_theme_query(selected_themes),
        build_theme_query(selected_themes),
    )

    if len(selected_themes) > 1:
        body += """
<div class="panel">
	<h2>Current theme scope</h2>
	<p>%s</p>
</div>
""" % h(selected_theme_text)

    return render_page("Redirecting...", body, extra_head)


def main():
    query = get_query()
    action = query.get("action", ["home"])[0].strip().lower()

    selected_themes, invalid_theme = get_selected_themes(query)
    if invalid_theme is not None:
        write_headers("400 Bad Request")
        sys.stdout.write(render_bad_request(invalid_theme))
        return

    if action == "list":
        write_headers("200 OK")
        sys.stdout.write(render_catalog(selected_themes))
        return

    if action == "pick":
        pool = filter_links(selected_themes)
        link = pick_random_link(pool)
        if link is None:
            write_headers("500 Internal Server Error")
            sys.stdout.write(render_no_result())
            return
        write_headers("200 OK")
        sys.stdout.write(render_redirect_landing(link, selected_themes))
        return

    write_headers("200 OK")
    sys.stdout.write(render_home(selected_themes))


if __name__ == "__main__":
    main()
