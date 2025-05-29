# visualizer.py
from models import InheritanceTree, ClassNode
from collections import defaultdict
from graphviz import Digraph


def display_tree(tree: InheritanceTree, show_inheritance_note: bool = True) -> None:
    parent_map = defaultdict(list)
    printed_once = set()

    # Build reverse map: class → parents
    for parent in tree.nodes.values():
        for child in parent.children:
            parent_map[child.name].append(parent.name)

    def print_node(node: ClassNode, prefix: str = "", is_last: bool = True):
        connector = "└── " if is_last else "├── "
        suffix = ""

        # Add inheritance note only for first printed appearance
        if show_inheritance_note and node.name not in printed_once:
            parents = parent_map[node.name]
            if len(parents) > 1:
                suffix = f" (inherits from: {', '.join(parents)})"
        printed_once.add(node.name)

        print(prefix + connector + node.name + suffix)

        child_count = len(node.children)
        for i, child in enumerate(node.children):
            is_last_child = (i == child_count - 1)
            child_prefix = prefix + ("    " if is_last else "│   ")
            print_node(child, child_prefix, is_last_child)

    # Find root classes
    all_children = {child.name for node in tree.nodes.values() for child in node.children}
    roots = [node for name, node in tree.nodes.items() if name not in all_children]

    for i, root in enumerate(roots):
        print_node(root, "", is_last=(i == len(roots) - 1))


def display_tree_graphviz(tree: InheritanceTree, output_file="inheritance_tree"):
    dot = Digraph(comment="C++ Inheritance Tree")
    dot.attr(rankdir="TB")

    # Add all class nodes
    for class_name in tree.nodes:
        dot.node(class_name)

    # Add edges based on parent → children
    for parent in tree.nodes.values():
        for child in parent.children:
            dot.edge(parent.name, child.name)

    # Save to file and open
    dot.render(output_file, view=False, format="pdf")
    print(f"Graph saved to {output_file}.pdf — open it manually.")

