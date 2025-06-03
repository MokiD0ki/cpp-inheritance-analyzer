import os
import pytest
from scanner import load_source
from parser import extract_classes
from tree_builder import build_inheritance_tree, validate_tree

SAMPLES_DIR = os.path.join("test", "samples")

def run_test_case(file_name, expected_edges=None, expected_warnings=None):
    full_path = os.path.join(SAMPLES_DIR, file_name)
    code = load_source(full_path)
    classes = extract_classes(code)
    tree = build_inheritance_tree(classes)

    # Collect declared and duplicate classes
    declared = getattr(extract_classes, "declared", set())
    duplicates = getattr(extract_classes, "duplicates", [])

    actual_warnings = validate_tree(tree, declared_class_names=declared)
    if duplicates:
        for dup in duplicates:
            actual_warnings.append(f"Duplicate class detected: {dup}")

    # Structural check
    if expected_edges:
        for parent, children in expected_edges.items():
            assert parent in tree.nodes
            for child in children:
                assert child in tree.nodes
                assert tree.nodes[child] in tree.nodes[parent].children

    # Warning check
    if expected_warnings is not None:
        for expected in expected_warnings:
            assert any(expected in msg for msg in actual_warnings), f"Expected warning not found: {expected}"



# === Test cases ===

def test_tc1_single_inheritance():
    run_test_case("tc1_single_inheritance.cpp", {
        "A": ["B"]
    })

def test_tc2_multiple_inheritance():
    run_test_case("tc2_multiple_inheritance.cpp", {
        "A": ["C"],
        "B": ["C"]
    })

def test_tc3_no_inheritance():
    run_test_case("tc3_no_inheritance.cpp", {
        # No relationships expected
    })

def test_tc4_deep_inheritance():
    run_test_case("tc4_deep_inheritance.cpp", {
        "A": ["B"],
        "B": ["C"],
        "C": ["D"]
    })

def test_tc5_out_of_order():
    run_test_case("tc5_out_of_order.cpp", {
        "A": ["B"]
    })

def test_tc6_mixed_declarations():
    run_test_case("tc6_mixed_declarations.cpp", {
        "A": ["B"]
    })

def test_tc7_access_specifiers():
    run_test_case("tc7_access_specifiers.cpp", {
        "A": ["B"]
    })

def test_tc8_diamond_inheritance():
    run_test_case("tc8_diamond_inheritance.cpp", {
        "A": ["B", "C"],
        "B": ["D"],
        "C": ["D"]
    })

def test_tc9_valid_header():
    run_test_case("tc9_valid_header.h", {
        "Animal": ["Dog", "Cat"]
    })

def test_tc10_inline_multiple():
    run_test_case("tc10_inline_multiple.cpp", {
        "A": ["B"],
        "B": ["C"]
    })

def test_tc11_empty():
    run_test_case("tc11_empty.cpp", {
        # No classes or edges expected
    })

def test_tc12_medium_size_inheritance():
    run_test_case("tc12_medium_size_inheritance.cpp", {
        "Animal": ["Mammal", "Bird"],
        "Mammal": ["Bat", "Whale", "Pegasus"],
        "Bird": ["Ostrich", "Pegasus"]
    })

def test_tc13_missing_base():
    run_test_case("tc13_missing_base.cpp", expected_warnings=[
        "inherited from but not defined"
    ])

def test_tc14_duplicate_classes():
    run_test_case("tc14_duplicate_classes.cpp", expected_warnings=[
        "Duplicate class detected"
    ])

def test_tc15_code_from_github():
    run_test_case("tc15_code_from_github.cpp", {
    "BufferDecoder": [
        "AbstractYuyvBufferDecoder",
        "CopyBufferDecoder",
        "JpegBufferDecoder"
    ],
    "AbstractYuyvBufferDecoder": [
        "SeparateYuyvBufferDecoder",
        "YuyvToGrayscaleBufferDecoder",
        "YuyvToRgbBufferDecoder"
    ]
    })
    
