from typing import List, Optional

class ClassInfo:
    def __init__(self, name: str, base_classes: List[str]):
        self.name = name
        self.base_classes = base_classes


class ClassNode:
    def __init__(self, name: str):
        self.name = name
        self.children: List['ClassNode'] = []


class InheritanceTree:
    def __init__(self):
        self.nodes = {}  # name -> ClassNode