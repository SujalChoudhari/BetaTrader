#!/usr/bin/env python3
import os
import re
import sys

def parse_gcov_file(filepath):
    """Parses a .gcov file and returns line coverage stats."""
    lines_total = 0
    lines_covered = 0
    
    try:
        with open(filepath, 'r', errors='ignore') as f:
            for line in f:
                # Format: "  count: line_num: content"
                # If count is "#####", it's uncovered.
                # If count is "-", it's non-executable.
                # If count is numeric > 0, it's covered.
                match = re.match(r'^\s*([^:]+):\s*([^:]+):', line)
                if match:
                    count_str = match.group(1).strip()
                    if count_str == '-':
                        continue
                    lines_total += 1
                    if count_str.isdigit() and int(count_str) > 0:
                        lines_covered += 1
    except Exception as e:
        print(f"Error reading {filepath}: {e}")
        return None
        
    return lines_total, lines_covered

def main():
    build_dir = "build/coverage"
    if not os.path.exists(build_dir):
        print(f"Build directory {build_dir} not found.")
        return

    gcov_files = []
    for root, _, files in os.walk(build_dir):
        for file in files:
            if file.endswith(".gcov"):
                gcov_files.append(os.path.join(root, file))

    if not gcov_files:
        print("No .gcov files found in build/coverage (excluding CMakeFiles directories).")
        return

    print(f"{'File':<50} | {'Lines':>6} | {'Covered':>7} | {'%':>6}")
    print("-" * 75)
    
    total_lines = 0
    total_covered = 0
    
    # Group by filename
    results = {}
    project_root = os.path.dirname(os.path.abspath(__file__))
    
    for filepath in gcov_files:
        basename = os.path.basename(filepath)
        
        # We want to check for the Source: line inside the gcov file to be sure it's ours
        try:
            with open(filepath, 'r', errors='ignore') as f:
                first_line = f.readline()
                if not first_line.startswith("        -:    0:Source:"):
                    continue
                source_path = first_line.split("Source:", 1)[1].strip()
                
                # Filter: Must be in project root, not in vendor, not in tests, not in build
                if not source_path.startswith(project_root): continue
                if "/vendor/" in source_path: continue
                if "/tests/" in source_path: continue
                if "build/" in source_path: continue
                
                # Get a nice display name
                display_name = os.path.relpath(source_path, project_root)
                
                stats = parse_gcov_file(filepath)
                if stats:
                    # Sum up stats if multiple gcov files refer to same source (unlikely but possible)
                    if display_name in results:
                        t, c = results[display_name]
                        results[display_name] = (t + stats[0], c + stats[1])
                    else:
                        results[display_name] = stats
        except Exception:
            continue

    for name in sorted(results.keys()):
        total, covered = results[name]
        percent = (covered / total * 100) if total > 0 else 0
        print(f"{name:<50} | {total:>6} | {covered:>7} | {percent:>5.1f}%")
        total_lines += total
        total_covered += covered

    print("-" * 75)
    total_percent = (total_covered / total_lines * 100) if total_lines > 0 else 0
    print(f"{'TOTAL':<50} | {total_lines:>6} | {total_covered:>7} | {total_percent:>5.1f}%")

if __name__ == "__main__":
    main()
