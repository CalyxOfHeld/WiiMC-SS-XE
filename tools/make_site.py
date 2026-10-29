#!/usr/bin/env python3
"""make_site.py [out dir]: the WiiMC-SS XE site (a small wiki) from the docs.

Each page of PAGES is rendered from its Markdown into docs/site/template.html,
with the sidebar of every page, "On this page" from its sections, links to
the pages before and after, and a search over every section of every page.
The home page is docs/site/home.html. The CSS and script are put in each
page, so the pages need nothing but the img/ folder beside them.

Needs markdown-it-py and mdit-py-plugins (pip install markdown-it-py
mdit-py-plugins). Default out dir: build/site.
"""
import html
import json
import os
import posixpath
import re
import shutil
import sys

from markdown_it import MarkdownIt
from mdit_py_plugins.anchors import anchors_plugin

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
SITE = os.path.join(ROOT, "docs", "site")
REPO_URL = "https://github.com/CalyxOfHeld/WiiMC-SS-XE"

# slug, source (from repository root), title, group, icon, description
PAGES = [
    ("index", None, "Home", "Start", "i-home",
     "The standalone media center for Nintendo Wii and vWii, enhanced with seamless WiiFin-XE integration."),
    ("getting-started", "docs/GETTING_STARTED.md", "Getting Started", "Start", "i-usb",
     "Installation, folder structure, and first launch instructions."),
    ("whats-new", "docs/XE_FEATURES.md", "What's New in XE", "Start", "i-star",
     "WiiFin-XE integration, local media handoff, and fail-safe DOL checks."),
    ("user-guide", "docs/USER_GUIDE.md", "User Guide", "Using it", "i-book",
     "Screens, playback options, artwork viewer, and controller mappings."),
    ("codecs", "docs/CODECS.md", "Codecs & Playback", "Using it", "i-play",
     "Supported video, audio, and subtitle formats, plus the .dash deblock skip trick."),
    ("faq", "docs/FAQ.md", "FAQ", "Using it", "i-help",
     "Answers to common questions and troubleshooting guide."),
    ("building", "docs/BUILDING.md", "Building", "Contributing", "i-code",
     "Compile WiiMC-SS XE from source with devkitPPC."),
]
SRC_TO_SLUG = {p[1]: p[0] for p in PAGES if p[1]}

def gh_slug(text):
    out = []
    for c in text.strip().lower():
        if c.isalnum() or c in "-_" or c == "?":
            out.append(c)
        elif c == " ":
            out.append("-")
    return "".join(out)

LEADING_SYMBOLS = re.compile(r"^(?:[\u2600-\u27bf]|[\U0001F000-\U0001FAFF]|\s)+")

def strip_tags(s):
    return html.unescape(re.sub(r"<[^>]+>", "", s))

def render_markdown(src):
    md = MarkdownIt("commonmark", {"html": True, "typographer": False}).enable("table")
    anchors_plugin(md, min_level=1, max_level=4, slug_func=gh_slug, permalink=False)
    with open(os.path.join(ROOT, src), encoding="utf-8") as f:
        text = f.read()
    return md.render(text)

def tidy_headings(body):
    """Leading emoji off the headings, and a # link beside each section's heading."""
    def fix(m):
        level, ident, inner = m.group(1), m.group(2), m.group(3)
        inner = LEADING_SYMBOLS.sub("", inner)
        link = ""
        if level in ("2", "3"):
            link = '<a class="anchor" href="#%s" aria-label="Link to this section">#</a>' % html.escape(ident, quote=True)
        return '<h%s id="%s">%s%s</h%s>' % (level, ident, inner, link, level)
    return re.sub(r'<h([1-6]) id="([^"]*)">(.*?)</h\1>', fix, body, flags=re.S)

def ids_of(body):
    return set(html.unescape(i) for i in re.findall(r'\sid="([^"]*)"', body))

def rewrite_links(body, src, problems, wanted):
    """Links to docs become links to pages; other repository files become GitHub links."""
    base = posixpath.dirname(src)

    def fix(m):
        attrs_before, href, attrs_after, text = m.group(1), m.group(2), m.group(3), m.group(4)
        raw = html.unescape(href)
        if re.match(r"^[a-z][a-z0-9+.-]*:", raw, re.I):
            return m.group(0)
        path, _, frag = raw.partition("#")
        if not path:
            wanted.append((src, src, frag))
            return m.group(0)
        target = posixpath.normpath(posixpath.join(base, path))
        if target in SRC_TO_SLUG:
            wanted.append((src, target, frag))
            dest = SRC_TO_SLUG[target] + ".html" + ("#" + frag if frag else "")
            return '<a%shref="%s"%s>%s</a>' % (attrs_before, dest, attrs_after, text)
        dest = REPO_URL + "/blob/master/" + target + ("#" + frag if frag else "")
        return '<a%shref="%s"%s>%s</a>' % (attrs_before, dest, attrs_after, text)

    return re.sub(r'<a\b([^>]*?)\bhref="([^"]*)"([^>]*)>(.*?)</a>', fix, body, flags=re.S)

def sections(body):
    parts = re.split(r'(?=<h[1-3]\b)', body)
    out = []
    for i in range(1, len(parts)):
        m = re.match(r'<h([1-3])\b[^>]*\bid="([^"]*)"[^>]*>(.*?)</h\1>', parts[i], re.S)
        if not m:
            continue
        heading = re.sub(r'<a class="anchor".*?</a>', "", m.group(3))
        out.append((int(m.group(1)), html.unescape(m.group(2)), strip_tags(heading).strip(), parts[i]))
    return out

def text_of(fragment, limit=700):
    t = strip_tags(re.sub(r"<(pre|table)\b.*?</\1>", lambda m: " " + strip_tags(m.group(0)) + " ", fragment, flags=re.S))
    t = re.sub(r"\s+", " ", t).strip()
    return t[:limit]

def sidebar(current):
    out, group = ['<aside class="sidebar"><details open><summary>All pages</summary>'], None
    for slug, _, title, grp, icon, _ in PAGES:
        if grp != group:
            if group is not None:
                out.append("</ul>")
            out.append("<h4>%s</h4><ul>" % html.escape(grp))
            group = grp
        cur = ' aria-current="page"' if slug == current else ""
        out.append('<li><a href="%s.html"%s><svg aria-hidden="true"><use href="#%s"/></svg>%s</a></li>'
                   % (slug, cur, icon, html.escape(title)))
    out.append("</ul></details></aside>")
    return "".join(out)

def toc(secs):
    items = ['<li><a href="#%s">%s</a></li>' % (html.escape(i, quote=True), html.escape(h))
             for lvl, i, h, _ in secs if lvl == 2]
    if not items:
        return ""
    return '<nav class="toc" aria-label="On this page"><h4>On this page</h4><ul>%s</ul></nav>' % "".join(items)

def pager(i):
    links = []
    if i > 0:
        p = PAGES[i - 1]
        links.append('<a class="prev" href="%s.html"><small>Previous</small><b>%s</b></a>' % (p[0], html.escape(p[2])))
    if i + 1 < len(PAGES):
        n = PAGES[i + 1]
        links.append('<a class="next" href="%s.html"><small>Next</small><b>%s</b></a>' % (n[0], html.escape(n[2])))
    return '<nav class="pager" aria-label="Pages">%s</nav>' % "".join(links)

def main():
    out_dir = sys.argv[1] if len(sys.argv) > 1 else os.path.join(ROOT, "build", "site")
    read = lambda p: open(os.path.join(SITE, p), encoding="utf-8").read()
    template, css, js = read("template.html"), read("site.css"), read("site.js")
    home = re.sub(r"^<!--.*?-->\s*", "", read("home.html"), flags=re.S)

    problems, wanted, ids, bodies, search = [], [], {}, {}, []
    for slug, src, title, *_ in PAGES:
        if not src:
            continue
        body = tidy_headings(render_markdown(src))
        body = rewrite_links(body, src, problems, wanted)
        bodies[slug] = body
        ids[src] = ids_of(body)
        for lvl, ident, heading, under in sections(body):
            text = text_of(under)
            if lvl == 1:
                search.append({"u": slug + ".html", "t": title, "h": title, "x": text})
            elif text or heading:
                search.append({"u": "%s.html#%s" % (slug, ident), "t": title, "h": heading, "x": text})

    for src, target, frag in wanted:
        if frag and posixpath.normpath(target) in ids:
            from urllib.parse import unquote
            if unquote(frag) not in ids[posixpath.normpath(target)]:
                problems.append("%s: no section #%s in %s" % (src, frag, target))

    home_ids = set(re.findall(r'href="([a-z-]+)\.html#([^"]+)"', home))
    for slug, frag in home_ids:
        src = next((p[1] for p in PAGES if p[0] == slug), None)
        from urllib.parse import unquote
        if src and unquote(frag) not in ids.get(src, set()):
            problems.append("docs/site/home.html: no section #%s in %s" % (frag, src))

    search_json = json.dumps(search, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")
    os.makedirs(out_dir, exist_ok=True)
    for i, (slug, src, title, group, icon, desc) in enumerate(PAGES):
        if src:
            body = bodies[slug]
            main_html = ('<div class="shell">%s<main class="doc" id="top"><article><p class="crumb">%s</p>%s%s</article></main>%s</div>'
                         % (sidebar(slug), html.escape(group), body, pager(i), toc(sections(body))))
            page_title = "%s · WiiMC-SS XE" % title
            body_class = "page"
        else:
            main_html = '<main class="home">%s</main>' % home
            page_title = "WiiMC-SS XE"
            body_class = "home"
        page = template
        for key, val in (("title", html.escape(page_title)), ("description", html.escape(desc, quote=True)),
                         ("body_class", body_class), ("css", css), ("js", js),
                         ("search_data", search_json), ("main", main_html)):
            page = page.replace("{{%s}}" % key, val)
        with open(os.path.join(out_dir, slug + ".html"), "w", encoding="utf-8", newline="\n") as f:
            f.write(page)

    shutil.copytree(os.path.join(SITE, "img"), os.path.join(out_dir, "img"), dirs_exist_ok=True)

    print("%d pages, %d search entries -> %s" % (len(PAGES), len(search), out_dir))
    for p in problems:
        print("broken link:", p)
    return 1 if problems else 0

if __name__ == "__main__":
    sys.exit(main())
