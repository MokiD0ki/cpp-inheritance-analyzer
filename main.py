import os
import questionary
from scanner import load_source
from parser import extract_classes
from tree_builder import build_inheritance_tree, validate_tree
from visualizer import display_tree, display_tree_graphviz

def find_cpp_files(folder: str) -> list[str]:
    return [
        os.path.join(folder, f)
        for f in os.listdir(folder)
        if f.endswith((".cpp", ".h")) and os.path.isfile(os.path.join(folder, f))
    ]

def load_multiple_sources(files: list[str]) -> str:
    return "\n".join(load_source(file) for file in files)

def main_loop():
    source_folder = "test/samples"  # Default folder for source files

    while True:
        files = sorted(find_cpp_files(source_folder))
        if not files:
            print("No .cpp or .h files found in folder:", source_folder)
            return

        selected_files = questionary.checkbox(
            "Select source file(s) to analyze:",
            choices=files
        ).ask()

        if not selected_files:
            print("No files selected. Exiting.")
            return

        code = load_multiple_sources(selected_files)
        classes = extract_classes(code)

        # Check for duplicate class definitions
        if hasattr(extract_classes, "duplicates") and extract_classes.duplicates:
            print("\n⚠ Duplicate Class Definitions:")
            for dup in extract_classes.duplicates:
                print(f"  [Warning] Duplicate class detected: '{dup}'")
            print("\n")

        tree = build_inheritance_tree(classes)

        # Fix: Include declared classes in validation
        declared = getattr(extract_classes, "declared", set())
        warnings = validate_tree(tree, declared_class_names=declared)
        if warnings:
            print("\n⚠ Validation Warnings:")
            for w in warnings:
                print("  " + w)
        else:
            print("\n✅ No structural issues found.")
        print("\n")

        output_mode = questionary.select(
            "Choose output mode:",
            choices=["Text (console)", "Graphviz (PDF)"]
        ).ask()
        print("\n")

        if output_mode == "Graphviz (PDF)":
            display_tree_graphviz(tree)
        else:
            note_mode = questionary.confirm("Show 'inherits from' notes for multi-inheritance?").ask()
            display_tree(tree, show_inheritance_note=note_mode)

        print("\n")
        again = questionary.confirm("Analyze another file?").ask()
        if not again:
            print("Goodbye!")
            break

if __name__ == "__main__":
    main_loop()
