import json

with open('glyphnames.json', 'r') as f:
    glyphs = json.load(f)

def codepoint_to_unicode(codepoint):
    return chr(codepoint)

def codepoint_to_utf8_escaped(cp: int) -> str:
    return ''.join(f'\\x{b:02x}' for b in chr(cp).encode('utf-8'))

with open('IconBindings.h', 'w') as f:
    f.write('#pragma once\n\n')
    for name, data in glyphs.items():
      if name == "METADATA": continue
      transformed_name = name.upper().replace(' ', '_').replace('-', '_')
      code = int(f"0x{data['code']}", 16)
      f.write(f'#define ICON_{transformed_name} "{codepoint_to_utf8_escaped(code)}" // {codepoint_to_unicode(code)}\n')