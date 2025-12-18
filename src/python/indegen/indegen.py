import argparse
import os
import re
import colorsys
import glob
from datetime import datetime

def hex_to_rgb(hex_str):
    hex_str = hex_str.lstrip('#')
    if len(hex_str) == 3:
        hex_str = ''.join([c*2 for c in hex_str])
    return tuple(int(hex_str[i:i+2], 16) for i in (0, 2, 4))

def rgb_to_hex(rgb):
    return '#%02x%02x%02x' % rgb

def get_dark_version(hex_str, lightness=0.15):
    """Returns a version of the color with fixed low lightness for backgrounds."""
    rgb = hex_to_rgb(hex_str)
    h, s, l = colorsys.rgb_to_hls(rgb[0]/255.0, rgb[1]/255.0, rgb[2]/255.0)
    r, g, b = colorsys.hls_to_rgb(h, lightness, s)
    return rgb_to_hex((int(r*255), int(g*255), int(b*255)))

def get_page_brand(filepath):
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        body = re.findall(r'body\s*\{[^}]*background(?:-color)?:\s*(#[0-9a-fA-F]{3,6})', content)
        vars = re.findall(r'--[\w-]+:\s*(#[0-9a-fA-F]{3,6})', content)
        points = []
        for c in body: points.append((hex_to_rgb(c), 10))
        for c in vars: points.append((hex_to_rgb(c), 5))
        if not points:
            general = re.findall(r'[:\s](#(?:[0-9a-fA-F]{3,6}))', content)
            for c in general: points.append((hex_to_rgb(c), 1))
        if not points: return None
        total_w = sum(p[1] for p in points)
        avg_r = sum(p[0][0] * p[1] for p in points) // total_w
        avg_g = sum(p[0][1] * p[1] for p in points) // total_w
        avg_b = sum(p[0][2] * p[1] for p in points) // total_w
        return rgb_to_hex((avg_r, avg_g, avg_b))
    except:
        return None

def generate_html(projects, args):
    cards_html = ""
    for p in projects:
        brand = p['brand'] if p['brand'] else args.accent
        dark_brand = get_dark_version(brand, lightness=args.card_darkness)
        
        cards_html += f"""
        <div class="card" style="--item-accent: {brand}; --item-bg: {dark_brand}">
            <h2 class="title">{p['name']}</h2>
            <div class="card-actions">
                <a href="{p['file']}" class="main-link">Open</a>
                {f'<a href="{p["docs_file"]}" class="docs-btn">{args.docs_label}</a>' if p['docs_file'] else ""}
            </div>
            <div class="footer">
                <span class="path">{p['file']}</span>
                <span class="date">{p['date']}</span>
            </div>
        </div>"""

    return f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{args.title}</title>
    <style>
        :root {{
            --bg: {args.bg};
            --fg: {args.fg};
            --accent-default: {args.accent};
            --border-w: {args.border_width}px;
            --gap: {args.gap}px;
            --grid-min: {args.width}px;
        }}
        * {{ box-sizing: border-box; }}
        body {{ 
            font-family: {args.font}; background-color: var(--bg); color: var(--fg);
            margin: 0; padding: {args.padding}rem; line-height: 1.0;
            -webkit-font-smoothing: antialiased; 
            background-image: url('{args.bg_img}'); background-repeat: repeat; 
            background-size: {args.bg_size}; background-attachment: fixed;
        }}
        .container {{ max-width: {args.max_width}px; margin: 0 auto; }}
        header {{ margin-bottom: 3rem; border-bottom: var(--border-w) solid var(--fg); padding-bottom: 4px; }}
        h1 {{ font-size: 2rem; font-weight: 900; margin: 0; }}
        
        .grid {{ display: grid; grid-template-columns: repeat(auto-fill, minmax(var(--grid-min), 1fr)); gap: var(--gap); }}
        
        .card {{
            border: var(--border-w) solid var(--item-accent); 
            padding: 0.5rem;
            display: flex; flex-direction: column; justify-content: space-between;
            background: var(--item-bg); transition: transform 0.5s ease;
            color: #FFFFFF;
        }}
        .card:hover {{ transform: translateY(-2px); }}

        .title {{ 
            font-size: 1.5rem; font-weight: 800; margin: 0.0rem 0 1.0rem 0; 
            line-height: 1.0; color: var(--item-accent); /* Matched to border colour */
        }}
        
        .card-actions {{ display: flex; gap: 0.5rem; margin-bottom: 1rem; }}
        .main-link, .docs-btn {{
            text-decoration: none; font-size: 0.7rem; font-weight: 900;
            padding: 0.25rem 0.25rem; border: 2px solid var(--fg); text-transform: uppercase;
            transition: all 0.5s;
        }}
        .main-link {{ border-color: var(--item-accent); color: var(--item-accent); }}
        .main-link:hover {{ background: var(--item-accent); color: var(--item-bg); }}
        .docs-btn {{ color: #FFFFFF; border-color: #FFFFFF; opacity: 0.6; }}
        .docs-btn:hover {{ opacity: 1; background: #FFFFFF; color: var(--item-bg); }}

        .footer {{ 
            display: flex; justify-content: space-between; align-items: flex-end;
            padding-top: 4px; border-top: 2px solid rgba(128,128,128,0.15);
            font-size: 1.0rem; font-family: monospace;
        }}
        .path, .date {{ opacity: 0.5; color: #FFFFFF; }}
    </style>
</head>
<body>
    <div class="container">
        <header><h1>{args.title}</h1></header>
        <div class="grid">{cards_html}</div>
    </div>
</body>
</html>
"""

def main():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("input", nargs="*", default=["*.html"])
    parser.add_argument("-o", "--output", default="index.html")
    parser.add_argument("-t", "--title", default="INDEX")
    
    parser.add_argument("-bg", "--background", dest="bg", default="#FFFFFF")
    parser.add_argument("-fg", "--foreground", dest="fg", default="#000000")
    parser.add_argument("-a", "--accent", default="#000000")
    parser.add_argument("-bw", "--border-width", type=int, default=4)
    parser.add_argument("-f", "--font", default='Helvetica, Arial, sans-serif')
    parser.add_argument("-p", "--padding", type=float, default=2.0)
    parser.add_argument("-w", "--width", type=int, default=240)
    parser.add_argument("-m", "--max-width", type=int, default=720)
    parser.add_argument("-g", "--gap", type=int, default=16)
    
    parser.add_argument("-bi", "--bg-img", default="tile.bmp")
    parser.add_argument("-bs", "--bg-size", default="auto")
    parser.add_argument("-cd", "--card-darkness", type=float, default=0.15)

    parser.add_argument("-ms", "--man-suffix", default="_man")
    parser.add_argument("-dl", "--docs-label", default="MAN")

    args = parser.parse_args()

    raw_files = []
    for pattern in args.input: raw_files.extend(glob.glob(pattern))
    all_files = list(set([f for f in raw_files if os.path.isfile(f) and f != args.output]))
    main_files = [f for f in all_files if not f.endswith(f"{args.man_suffix}.html")]
    
    project_data = []
    for f in main_files:
        base = os.path.splitext(f)[0]
        man_file = f"{base}{args.man_suffix}.html"
        project_data.append({
            'file': f,
            'docs_file': man_file if os.path.exists(man_file) else None,
            'brand': get_page_brand(f),
            'name': base.replace('-', ' ').replace('_', ' ').upper(),
            'date': datetime.fromtimestamp(os.path.getmtime(f)).strftime('%y.%m.%d')
        })

    project_data.sort(key=lambda x: x['name'])
    with open(args.output, "w", encoding='utf-8') as out:
        out.write(generate_html(project_data, args))
    
    print(f"✨ Index generated with themed titles: {args.output}")

if __name__ == "__main__":
    main()
