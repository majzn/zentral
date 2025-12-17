import argparse
import os

def generate_html(files, title, description):
    # Modern Heroic Template with Glassmorphism effects
    template = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>{title}</title>
    <script src="https://cdn.tailwindcss.com"></script>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&display=swap" rel="stylesheet">
    <style>
        body {{ font-family: 'Inter', sans-serif; background-color: #0f172a; color: #f8fafc; }}
        .glass {{ background: rgba(255, 255, 255, 0.03); backdrop-filter: blur(10px); border: 1px solid rgba(255, 255, 255, 0.1); transition: all 0.3s ease; }}
        .glass:hover {{ background: rgba(255, 255, 255, 0.08); border-color: rgba(255, 255, 255, 0.2); transform: translateY(-4px); }}
    </style>
</head>
<body class="min-h-screen py-12 px-4">
    <div class="max-w-5xl mx-auto">
        <header class="mb-16 text-center">
            <h1 class="text-5xl font-extrabold mb-4 bg-clip-text text-transparent bg-gradient-to-r from-blue-400 to-emerald-400">
                {title}
            </h1>
            <p class="text-slate-400 text-lg max-w-2xl mx-auto">
                {description}
            </p>
        </header>

        <div class="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
    """

    for file in files:
        # Clean up the display name (remove .html, replace dashes with spaces)
        display_name = os.path.splitext(file)[0].replace('-', ' ').replace('_', ' ').title()
        
        template += f"""
            <a href="{file}" class="glass p-6 rounded-2xl flex flex-col justify-between group">
                <div>
                    <div class="w-10 h-10 mb-4 rounded-lg bg-blue-500/20 flex items-center justify-center text-blue-400 group-hover:bg-blue-500 group-hover:text-white transition-colors">
                        <svg xmlns="http://www.w3.org/2000/svg" class="h-6 w-6" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                          <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
                        </svg>
                    </div>
                    <h2 class="text-xl font-semibold mb-2 text-white">{display_name}</h2>
                    <p class="text-slate-500 text-sm italic">{file}</p>
                </div>
                <div class="mt-8 flex items-center text-blue-400 font-medium text-sm">
                    View Project 
                    <svg xmlns="http://www.w3.org/2000/svg" class="h-4 w-4 ml-1 transform group-hover:translate-x-1 transition-transform" fill="none" viewBox="0 0 24 24" stroke="currentColor">
                      <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M14 5l7 7m0 0l-7 7m7-7H3" />
                    </svg>
                </div>
            </a>
        """

    template += """
        </div>

        <footer class="mt-20 pt-8 border-t border-slate-800 text-center text-slate-500 text-sm">
            Generated with Python & Tailwind CSS
        </footer>
    </div>
</body>
</html>
    """
    return template

def main():
    parser = argparse.ArgumentParser(description="Generate a modern index.html for your projects.")
    parser.add_argument("files", nargs="+", help="The HTML files to include in the index.")
    parser.add_argument("--title", default="Project Hub", help="Title for the page.")
    parser.add_argument("--desc", default="A collection of my web experiments and projects.", help="Description for the page.")
    parser.add_argument("--output", default="index.html", help="Output filename.")

    args = parser.parse_args()

    html_content = generate_html(args.files, args.title, args.desc)
    
    with open(args.output, "w") as f:
        f.write(html_content)
    
    print(f"✅ Successfully generated {args.output} with {len(args.files)} entries.")

if __name__ == "__main__":
    main()
