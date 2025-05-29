import re
from models import ClassInfo
from typing import List

def extract_classes(source_code: str) -> List[ClassInfo]:
    from collections import Counter
    import re
    from models import ClassInfo

    CLASS_REGEX = re.compile(r'class\s+(\w+)\s*(?:\:\s*([^{};]+))?\s*\{')

    class_infos = []
    class_names = []

    for match in CLASS_REGEX.finditer(source_code):
        class_name = match.group(1)
        base_part = match.group(2)
        bases = []
        if base_part:
            bases = [part.strip().split()[-1] for part in base_part.split(',')]
        class_infos.append(ClassInfo(class_name, bases))
        class_names.append(class_name)

    # Track duplicate names
    duplicates = [name for name, count in Counter(class_names).items() if count > 1]
    extract_classes.duplicates = duplicates  # Save for validator
    extract_classes.declared = set(class_names)

    return class_infos
