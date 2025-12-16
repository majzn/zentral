import os
import shutil
import subprocess
import sys
import shlex
import argparse 
import datetime
import logging # For logging functionality
import functools # For binding print to logger

# Attempt to import tqdm for progress bars (optional dependency)
try:
    from tqdm import tqdm
except ImportError:
    tqdm = None
    print("WARNING: tqdm not found. Install with 'pip install tqdm' for progress bars.")

# --- ANSI Color Codes (for aesthetic output) ---
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

# List of all ANSI color codes we use, for stripping without 're'
# This is simpler and avoids the dependency.
ALL_COLOR_CODES = [
    Colors.HEADER, Colors.OKBLUE, Colors.OKGREEN, Colors.WARNING, 
    Colors.FAIL, Colors.ENDC, Colors.BOLD, Colors.UNDERLINE
]

# --- Configuration ---
BUILD_DIR = 'build'
CONFIG_FILE = 'kraft.txt'
CURRENT_YEAR = datetime.datetime.now().year
DEFAULT_AUTHOR = "[AUTHOR]"
DEFAULT_PROJECT = "[PROJECT]"

# --- Logging Setup ---

# Custom Handler to strip ANSI colors based on the predefined list
class ColorStrippingFileHandler(logging.FileHandler):
    def emit(self, record):
        # Store original message and strip colors for the file log
        original_msg = record.msg
        stripped_msg = str(record.msg)
        
        for code in ALL_COLOR_CODES:
            stripped_msg = stripped_msg.replace(code, '')
        
        record.msg = stripped_msg
        super().emit(record)
        
        # Restore the original message for other handlers (like the console)
        record.msg = original_msg

# We use the built-in logger to manage output streams
logger = logging.getLogger('KraftLogger')
logger.setLevel(logging.INFO)

# --- Template Generation Functions ---

def get_template_readme(project_name, author_name):
    """Generates the README content using provided metadata."""
    return f"""# {project_name}

A brief, one-sentence description of your project.

## Copyright and Licensing

This project is fully copyrighted by {author_name}, and all rights are reserved.
No public rights are granted for copying, modifying, or distributing this software.
See the LICENSE file for details on usage restrictions.

## Build with Kraft

This project uses the {Colors.BOLD}Kraft{Colors.ENDC} build tool, configured via the `{CONFIG_FILE}` file.
"""

def get_template_license(author_name, license_year):
    """Generates the restrictive LICENSE content using provided metadata."""
    return f"""
All Rights Reserved

Copyright (c) {license_year} {author_name}

This software and related documentation are the proprietary property of {author_name} 
and are protected by copyright laws and international treaties.

Unauthorized copying, modification, redistribution, or use of this software, 
in whole or in part, is strictly prohibited. No rights are granted hereunder 
except for the right to view the source code.
"""

TEMPLATE_KRAFTFILE = """# kraft.txt - build configuration.
# Use space separation for file patterns and arguments.
# Use quotes for paths or arguments containing spaces.

# 1. GATHER: Copy all necessary source files and assets to the 'build' directory.
# Example for a C/C++ project:
gather src/*.c src/*.h

# 2. PREPROCESS: Run a script (e.g., generating version headers) before the main build.
# preprocess ../scripts/generate_version.py

# 3. BUILD: Execute the main compilation or packaging command inside the 'build' directory.
build gcc -o my_app main.c

# 4. POSTPROCESS: Run a script (e.g., zipping files, cleaning up) after the build.
# postprocess zip -r ../my_app.zip .

# 5. RUN: Execute the final built artifact.
run ./my_app
"""

# --- UI Functions ---

def log_and_print(level, message):
    """Logs the message using the logger."""
    if level == 'INFO':
        logger.info(message)
    elif level == 'WARNING':
        logger.warning(message)
    elif level == 'ERROR':
        logger.error(message)
    else:
        logger.info(message)

def print_header(title, color=Colors.HEADER):
    log_and_print('INFO', f"\n{color}{'='*40}{Colors.ENDC}")
    log_and_print('INFO', f"{color}{' '*3}{Colors.BOLD}{title.center(34)}{Colors.ENDC}")
    log_and_print('INFO', f"{color}{'='*40}{Colors.ENDC}")

def print_step_header(step_name, step_index):
    log_and_print('INFO', f"\n{Colors.OKBLUE}--- Step {step_index}: {Colors.BOLD}{step_name}{Colors.ENDC}{Colors.OKBLUE} ---{Colors.ENDC}")

# --- Core Build Utilities ---

def clean_build():
    if os.path.exists(BUILD_DIR):
        log_and_print('WARNING', f"\n{Colors.WARNING}Cleaning up '{BUILD_DIR}'...{Colors.ENDC}")
        shutil.rmtree(BUILD_DIR)

def ensure_build_dir():
    os.makedirs(BUILD_DIR, exist_ok=True)
    log_and_print('INFO', f"{Colors.OKGREEN}Target Directory: '{BUILD_DIR}'{Colors.ENDC}")

def load_config(config_file):
    if not os.path.exists(config_file):
        log_and_print('ERROR', f"{Colors.FAIL}Error: Configuration file '{config_file}' not found.{Colors.ENDC}")
        sys.exit(1)
    
    with open(config_file, 'r', encoding='utf-8') as f:
        lines = [line.strip() for line in f if line.strip() and not line.startswith('#')]
    return lines

def execute_kraft(kraft_commands):
    for i, command in enumerate(kraft_commands):
        
        parts = command.split(maxsplit=1)
        if not parts: continue

        action = parts[0].lower()
        args_string = parts[1].strip() if len(parts) > 1 else ''
        
        print_step_header(command, i + 1)

        try:
            args_list = shlex.split(args_string)

            if action == 'gather':
                patterns = args_list
                if not patterns:
                    log_and_print('WARNING', f"{Colors.WARNING}Warning: 'gather' requires file patterns. Skipping.{Colors.ENDC}")
                    continue
                
                # 1. Pre-scan for total files
                potential_files = []
                for root, _, files in os.walk('.'):
                    if root.startswith(f'./{BUILD_DIR}'): continue
                    for file in files:
                        full_path = os.path.join(root, file)
                        is_match = False
                        for pattern in patterns:
                            if full_path == pattern or os.path.basename(full_path) == pattern: is_match = True; break
                            if pattern.startswith('*') and os.path.basename(full_path).endswith(pattern[1:]): is_match = True; break
                        if is_match:
                            potential_files.append((root, file))

                total_files = len(potential_files)
                copied_count = 0
                
                if total_files == 0:
                    log_and_print('INFO', " -> No files matched the gather patterns.")
                
                # 2. Iterate and copy with tqdm (only if tqdm is available)
                iterable = potential_files
                if tqdm:
                    iterable = tqdm(potential_files, desc=f"{Colors.OKBLUE}Gathering Files{Colors.ENDC}", unit=" files", bar_format="{l_bar}{bar}| {n_fmt}/{total_fmt} [{elapsed}<{remaining}, {rate_fmt}]")
                
                for root, file in iterable:
                    full_path = os.path.join(root, file)
                    rel_dir = os.path.relpath(root, '.')
                    dest_dir = os.path.join(BUILD_DIR, rel_dir)
                    os.makedirs(dest_dir, exist_ok=True)
                    shutil.copy2(full_path, os.path.join(dest_dir, file))
                    copied_count += 1
                
                log_and_print('INFO', f"{Colors.OKGREEN}Gathered {copied_count} file(s).{Colors.ENDC}")
                
            elif action in ['run', 'build', 'preprocess', 'postprocess']:
                if not args_list:
                    raise ValueError(f"'{action}' command requires arguments.")

                command_list = args_list
                
                log_and_print('INFO', f"    -> {Colors.BOLD}Command: {Colors.ENDC}{' '.join(command_list)}")
                log_and_print('INFO', f"    -> Running in directory: {Colors.UNDERLINE}{BUILD_DIR}{Colors.ENDC}")
                
                subprocess.run(
                    command_list,
                    check=True,
                    cwd=BUILD_DIR,
                    stdout=sys.stdout,
                    stderr=sys.stderr
                )
                log_and_print('INFO', f"{Colors.OKGREEN}{action.capitalize()} complete.{Colors.ENDC}")
                
            else:
                log_and_print('WARNING', f"{Colors.WARNING}Warning: Unknown action '{action}'. Skipping.{Colors.ENDC}")
                
        except subprocess.CalledProcessError as e:
            log_and_print('ERROR', f"\n{Colors.FAIL}!!! Kraft Failed !!! Command returned non-zero exit code {e.returncode}.{Colors.ENDC}")
            log_and_print('ERROR', f"{Colors.FAIL}Command: {args_list}{Colors.ENDC}")
            sys.exit(e.returncode)
        except FileNotFoundError:
            log_and_print('ERROR', f"\n{Colors.FAIL}!!! Kraft Failed !!! Command not found: {args_list[0]}.{Colors.ENDC}")
            sys.exit(1)
        except Exception as e:
            log_and_print('ERROR', f"\n{Colors.FAIL}!!! Kraft Failed !!! due to an internal error: {e}{Colors.ENDC}")
            sys.exit(1)


# --- Scaffolding Functions ---
def generate_file(filename, content):
    if os.path.exists(filename):
        log_and_print('WARNING', f"  {Colors.WARNING}'{filename}' already exists. Skipping.{Colors.ENDC}")
        return False
    
    with open(filename, 'w', encoding='utf-8') as f:
        f.write(content)
    log_and_print('INFO', f"  {Colors.OKGREEN}Created file: '{filename}'{Colors.ENDC}")
    return True

def init_config():
    log_and_print('INFO', f"{Colors.OKBLUE}Initializing {CONFIG_FILE}...{Colors.ENDC}")
    generate_file(CONFIG_FILE, TEMPLATE_KRAFTFILE)

def init_docs(project_name, author_name, license_year):
    log_and_print('INFO', f"{Colors.OKBLUE}Initializing Documentation...{Colors.ENDC}")
    
    readme_content = get_template_readme(project_name, author_name)
    license_content = get_template_license(author_name, license_year)
    
    generate_file('README.md', readme_content)
    generate_file('LICENSE', license_content) 

def init_structure():
    log_and_print('INFO', f"{Colors.OKBLUE}Initializing Directory Structure...{Colors.ENDC}")
    dirs = ['src', 'bin', 'res']
    
    created = False
    for d in dirs:
        if not os.path.exists(d):
            os.makedirs(d)
            log_and_print('INFO', f"  {Colors.OKGREEN}Created directory: '{d}'{Colors.ENDC}")
            created = True
        else:
            log_and_print('WARNING', f"  {Colors.WARNING}Directory '{d}' already exists. Skipping.{Colors.ENDC}")

    if not created:
        log_and_print('INFO', f"  {Colors.OKBLUE}Note: All standard directories already existed.{Colors.ENDC}")


# --- Main Execution and Argument Parsing ---

def setup_logging(log_filename=None):
    """Sets up dual logging output to console and optional file."""
    
    # 1. Console Handler (Keeps colors)
    console_handler = logging.StreamHandler(sys.stdout)
    # Simple formatter for the console output (no timestamp/level, relies on function wrappers)
    console_formatter = logging.Formatter('%(message)s')
    console_handler.setFormatter(console_formatter)
    logger.addHandler(console_handler)

    if log_filename:
        
        # Add timestamp to the log file name
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d_%H-%M-%S")
        final_log_filename = f"{os.path.splitext(log_filename)[0]}_{timestamp}.log"
        
        # 2. File Handler (Strips colors and adds detailed timestamps)
        # Using the custom ColorStrippingFileHandler
        file_handler = ColorStrippingFileHandler(final_log_filename, encoding='utf-8')
        
        # Detailed formatter for the file log
        file_formatter = logging.Formatter('%(asctime)s | %(levelname)-8s | %(message)s', datefmt='%Y-%m-%d %H:%M:%S')
        file_handler.setFormatter(file_formatter)
        logger.addHandler(file_handler)
        
        log_and_print('INFO', f"{Colors.OKBLUE}Logging output to: {final_log_filename}{Colors.ENDC}")
    
    # Override standard print behavior for the logger
    global print
    print = functools.partial(log_and_print, 'INFO')


def main():
    parser = argparse.ArgumentParser(
        description=f"{Colors.BOLD}Kraft:{Colors.ENDC} Build tool for retards",
        epilog=f"To run the build, use: {Colors.BOLD}python {os.path.basename(__file__)}{Colors.ENDC}"
    )
    
    # --- Logging Group ---
    log_group = parser.add_argument_group(f'{Colors.OKBLUE}Logging Options{Colors.ENDC}', 'Configure detailed log file generation.')
    log_group.add_argument(
        '--log-file', '-l',
        type=str,
        nargs='?', 
        const='kraft_build', 
        help="Generate a detailed, timestamped log file. Optionally provide a base filename (default: kraft_build)."
    )

    # --- Initialization Options Group ---
    init_group = parser.add_argument_group(f'{Colors.OKBLUE}Initialization Options{Colors.ENDC}', 'Use these options to scaffold a new project.')
    init_group.add_argument('--init-config', '-c', action='store_true', help=f"Generate a template {CONFIG_FILE}.")
    init_group.add_argument('--init-structure', '-s', action='store_true', help="Generate a generic 'src/', 'tests/', and 'scripts/' directory structure.")
    init_group.add_argument('--init-docs', '-d', action='store_true', help="Generate template README.md and LICENSE files ('All Rights Reserved').")
    init_group.add_argument('--init-all', '-a', action='store_true', help="Generate ALL templates (config, structure, and docs).")
    
    # --- Metadata Options Group ---
    meta_group = parser.add_argument_group(f'{Colors.OKBLUE}Metadata Options{Colors.ENDC}', 'Use these to customize generated files.')
    meta_group.add_argument(
        '--project-name', '-n', type=str, default=DEFAULT_PROJECT,
        help="The name of the project to include in the README."
    )
    meta_group.add_argument(
        '--author-name', '-u', type=str, default=DEFAULT_AUTHOR,
        help="The author's full name for copyright/license headers."
    )
    meta_group.add_argument(
        '--license-year', '-y', type=int, default=CURRENT_YEAR,
        help=f"The copyright year for the LICENSE file (default: {CURRENT_YEAR})."
    )

    args = parser.parse_args()

    # --- Setup Logging First ---
    setup_logging(args.log_file)


    # Determine if any scaffolding is requested
    is_scaffolding = args.init_config or args.init_structure or args.init_docs or args.init_all

    if is_scaffolding:
        print_header("Kraft Scaffolding Engine", Colors.OKBLUE)
        
        project_name = args.project_name
        author_name = args.author_name
        license_year = args.license_year

        # Handle --init-all or specific flags
        if args.init_all:
            init_config()
            init_structure()
            init_docs(project_name, author_name, license_year)
        else:
            if args.init_config: init_config()
            if args.init_structure: init_structure()
            
            if args.init_docs: 
                init_docs(project_name, author_name, license_year)
            
        log_and_print('INFO', f"\n{Colors.OKGREEN}Scaffolding Complete!{Colors.ENDC}")
        return 

    # --- Handle Normal Build Request ---
    print_header("Kraft Build Tool", Colors.HEADER)

    clean_build()
    ensure_build_dir()
    
    commands = load_config(CONFIG_FILE)
    if not commands:
        log_and_print('INFO', f"{Colors.OKBLUE} Info: {CONFIG_FILE} is empty. Nothing to build.{Colors.ENDC}")
        return

    execute_kraft(commands)

    log_and_print('INFO', f"\n{Colors.OKGREEN}{'='*40}{Colors.ENDC}")
    log_and_print('INFO', f"{Colors.OKGREEN}{' '*3}{Colors.BOLD}BUILD SUCCESSFUL!{Colors.ENDC}{Colors.OKGREEN}{' '*3}{Colors.ENDC}")
    log_and_print('INFO', f"{Colors.OKGREEN}{'='*40}{Colors.ENDC}")


if __name__ == '__main__':
    main()
