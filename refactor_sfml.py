import os
import re

def process_file(filepath):
    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    original_content = content

    # 1. Replace specific SFML includes with our custom one
    content = re.sub(r'#include\s+<SFML/Graphics[^>]*>', '#include "joanna/core/graphics.h"', content)
    content = re.sub(r'#include\s+<SFML/Audio[^>]*>', '#include "joanna/core/audio.h"', content)
    content = re.sub(r'#include\s+<SFML/Window[^>]*>', '#include "joanna/core/graphics.h"', content)
    content = re.sub(r'#include\s+<SFML/System[^>]*>', '#include "joanna/core/graphics.h"', content)

    # Clean up duplicate graphics.h includes that might arise
    content = re.sub(r'(#include "joanna/core/graphics\.h"\n)+', '#include "joanna/core/graphics.h"\n', content)

    # 2. Replace the sf:: namespace with jo::
    content = re.sub(r'\bsf::', 'jo::', content)

    if content != original_content:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        print(f"Updated {filepath}")

def main():
    base_dir = '/home/johannes/Desktop/playground/joannas_adventure'
    dirs_to_process = [os.path.join(base_dir, 'src'), os.path.join(base_dir, 'include')]
    
    for dir_path in dirs_to_process:
        for root, _, files in os.walk(dir_path):
            for file in files:
                if file.endswith(('.cpp', '.h')):
                    process_file(os.path.join(root, file))

if __name__ == '__main__':
    main()
