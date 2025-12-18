import os
import shutil
import subprocess
import argparse
import logging
import sys
import json

def setup_logging(log_level):
    logging.basicConfig(
        level=log_level,
        format="%(asctime)s [%(levelname)s] %(message)s",
        handlers=[logging.FileHandler("deploy.log"), logging.StreamHandler(sys.stdout)]
    )

def parse_kv_pairs(pairs):
    if not pairs: return {}
    result = {}
    for item in pairs:
        if '=' in item:
            k, v = item.split('=', 1)
            result[k.strip()] = v.strip()
    return result

def run_deploy(args):
    try:
        config = {}
        if args.config:
            with open(args.config, 'r') as f:
                config = json.load(f)

        source = args.source or config.get("source", ".")
        target = args.target or config.get("target", "dist")
        
        # Priority: CLI KV pairs -> CLI JSON -> Config File
        files_to_copy = parse_kv_pairs(args.files_kv)
        if not files_to_copy:
            files_to_copy = json.loads(args.files_json) if args.files_json else config.get("files", {})
        
        build_cmds = args.commands or config.get("commands", [])

        if not os.path.isdir(source):
            logging.error(f"Source not found: {source}")
            sys.exit(1)

        if args.clean and os.path.exists(target):
            logging.info(f"Cleaning: {target}")
            shutil.rmtree(target)
        
        os.makedirs(target, exist_ok=True)

        for src_name, dest_name in files_to_copy.items():
            src_path = os.path.join(source, src_name)
            dest_path = os.path.join(target, dest_name)
            if os.path.exists(src_path):
                os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                shutil.copy2(src_path, dest_path)
                logging.info(f"Copied: {src_name} -> {dest_name}")

        if not args.dry_run:
            for cmd in build_cmds:
                logging.info(f"Running: {cmd}")
                subprocess.run(cmd, shell=True, check=True)

        logging.info("Deployment successful.")

    except Exception as e:
        logging.error(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--source", default=".")
    parser.add_argument("-t", "--target", default="dist")
    parser.add_argument("-f", "--files-kv", nargs='*', help="Pairs like file.html=index.html")
    parser.add_argument("-fj", "--files-json", help="JSON string for complex mappings")
    parser.add_argument("-x", "--commands", nargs='*', help="Build commands as strings")
    parser.add_argument("-j", "--config")
    parser.add_argument("-c", "--clean", action="store_true")
    parser.add_argument("-d", "--dry-run", action="store_true")
    parser.add_argument("-l", "--log", default="INFO")
    
    args = parser.parse_args()
    setup_logging(args.log)
    run_deploy(args)mport os
import shutil
import subprocess
import argparse
import logging
import sys
import json

def setup_logging(log_level):
    logging.basicConfig(
        level=log_level,
        format="%(asctime)s [%(levelname)s] %(message)s",
        handlers=[logging.FileHandler("deploy.log"), logging.StreamHandler(sys.stdout)]
    )

def parse_kv_pairs(pairs):
    if not pairs: return {}
    result = {}
    for item in pairs:
        if '=' in item:
            k, v = item.split('=', 1)
            result[k.strip()] = v.strip()
    return result

def run_deploy(args):
    try:
        config = {}
        if args.config:
            with open(args.config, 'r') as f:
                config = json.load(f)

        source = args.source or config.get("source", ".")
        target = args.target or config.get("target", "dist")
        
        # Priority: CLI KV pairs -> CLI JSON -> Config File
        files_to_copy = parse_kv_pairs(args.files_kv)
        if not files_to_copy:
            files_to_copy = json.loads(args.files_json) if args.files_json else config.get("files", {})
        
        build_cmds = args.commands or config.get("commands", [])

        if not os.path.isdir(source):
            logging.error(f"Source not found: {source}")
            sys.exit(1)

        if args.clean and os.path.exists(target):
            logging.info(f"Cleaning: {target}")
            shutil.rmtree(target)
        
        os.makedirs(target, exist_ok=True)

        for src_name, dest_name in files_to_copy.items():
            src_path = os.path.join(source, src_name)
            dest_path = os.path.join(target, dest_name)
            if os.path.exists(src_path):
                os.makedirs(os.path.dirname(dest_path), exist_ok=True)
                shutil.copy2(src_path, dest_path)
                logging.info(f"Copied: {src_name} -> {dest_name}")

        if not args.dry_run:
            for cmd in build_cmds:
                logging.info(f"Running: {cmd}")
                subprocess.run(cmd, shell=True, check=True)

        logging.info("Deployment successful.")

    except Exception as e:
        logging.error(f"Error: {e}")
        sys.exit(1)

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-s", "--source", default=".")
    parser.add_argument("-t", "--target", default="dist")
    parser.add_argument("-f", "--files-kv", nargs='*', help="Pairs like file.html=index.html")
    parser.add_argument("-fj", "--files-json", help="JSON string for complex mappings")
    parser.add_argument("-x", "--commands", nargs='*', help="Build commands as strings")
    parser.add_argument("-j", "--config")
    parser.add_argument("-c", "--clean", action="store_true")
    parser.add_argument("-d", "--dry-run", action="store_true")
    parser.add_argument("-l", "--log", default="INFO")
    
    args = parser.parse_args()
    setup_logging(args.log)
    run_deploy(args)
