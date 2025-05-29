from models import InheritanceTree, ClassNode, ClassInfo
from typing import List

def build_inheritance_tree(classes: List[ClassInfo]) -> InheritanceTree:
    tree = InheritanceTree()
    defined = set()
    referenced = set()

    for cls in classes:
        defined.add(cls.name)
        if cls.name not in tree.nodes:
            tree.nodes[cls.name] = ClassNode(cls.name)
        for base in cls.base_classes:
            referenced.add(base)
            if base not in tree.nodes:
                tree.nodes[base] = ClassNode(base)
            tree.nodes[base].children.append(tree.nodes[cls.name])

    tree.defined = defined
    tree.referenced = referenced
    return tree



def validate_tree(tree: InheritanceTree, declared_class_names: set[str] = None) -> list[str]:
    warnings = []

    # Use referenced/defined info from tree
    if hasattr(tree, "referenced") and hasattr(tree, "defined"):
        for name in tree.referenced:
            if name not in tree.defined:
                warnings.append(f"[Warning] Class '{name}' is inherited from but not defined.")

    return warnings


