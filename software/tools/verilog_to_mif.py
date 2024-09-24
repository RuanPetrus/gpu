import sys
from dataclasses import dataclass


TEXT_BEGIN = 0x00400000
TEXT_SIZE = 64 * 1024
DATA_BEGIN = 0x10010000
DATA_SIZE = 160 * 1024

@dataclass
class Section:
    begin: int
    insts: list[str]

def parse_verilog(content: str) -> list[Section]:
    sections: list[Section] = []
    section: Optional[Section] = None
    for line in content.split("\n"):
        line = line.strip()
        if (len(line) == 0):
            continue
        if line.startswith("@"):
            if (section is not None):
                sections.append(section)
                
            begin = int(line[1:], 16)
            section = Section(begin=begin, insts=[])

        else:
            bs = line.split()
            insts = [
                bs[i] + bs[i+1] + bs[i+2] + bs[i+3]
                for i in range(0, len(bs), 4)
            ]
            section.insts.extend(insts)
    
    if (section is not None):
        sections.append(section)

    return sections

def generate_mif(output_name: str, sections: list[Section], memory_begin: int, memory_size: int):
    with open(output_name + ".mif", "w") as f:
        f.write(f"DEPTH = {memory_size // 2};\n")
        f.write("WIDTH = 32;\n")
        f.write("ADDRESS_RADIX = HEX;\n")
        f.write("DATA_RADIX = HEX;\n")
        f.write("CONTENT\n")
        f.write("BEGIN\n")

        for s in sections:
            s_begin = (s.begin - memory_begin) // 4
            for i, inst in enumerate(s.insts):
                addrr = hex(s_begin + i)[2:].zfill(8)
                f.write(f"{addrr} : {inst};\n")

        f.write("END;\n")

def main():
    file_name = sys.argv[1]
    with open(file_name, "r") as f:
        content = f.read()

    sections = parse_verilog(content)
    generate_mif(sys.argv[2] + "_text", [
        s
        for s in sections
        if s.begin >= TEXT_BEGIN and s.begin < TEXT_BEGIN + TEXT_SIZE
    ], TEXT_BEGIN, TEXT_SIZE)

    generate_mif(sys.argv[2] + "_data", [
        s
        for s in sections
        if s.begin >= DATA_BEGIN and s.begin < DATA_BEGIN + DATA_SIZE
    ], DATA_BEGIN, DATA_SIZE)

if __name__ == "__main__":
    main()
