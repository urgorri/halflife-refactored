#!/usr/bin/env python3
"""
Behavioral Equivalence Verification - Layer 4: Exported Symbol Table Verification
Ensures that refactoring does not remove, rename, or drop required exported entity factory
functions, DLL initialization points, or API callbacks.

Usage:
  # Check Windows Server DLL exports
  python tests/verify_symbols.py --golden tests/golden/symbols_server_windows.txt --binary projects/vs2019/Release/hldll/hl.dll

  # Check from a dump file (e.g. dumpbin /exports output or nm -D output)
  python tests/verify_symbols.py --golden tests/golden/symbols_server_windows.txt --current dump.txt

  # Update golden baseline
  python tests/verify_symbols.py --golden tests/golden/symbols_server_windows.txt --binary projects/vs2019/Release/hldll/hl.dll --update
"""

import argparse
import os
import re
import shutil
import struct
import subprocess
import sys


def find_dumpbin():
    """Find dumpbin.exe in PATH or Visual Studio installations."""
    which = shutil.which("dumpbin")
    if which:
        return which

    # Try vswhere if available
    vswhere = os.path.expandvars(r"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe")
    if os.path.exists(vswhere):
        try:
            out = subprocess.check_output(
                [vswhere, "-latest", "-find", r"**\dumpbin.exe"],
                text=True,
                stderr=subprocess.DEVNULL
            ).strip().splitlines()
            if out and os.path.exists(out[0]):
                return out[0]
        except Exception:
            pass

    # Common fallbacks
    for base in [
        r"C:\Program Files\Microsoft Visual Studio",
        r"C:\Program Files (x86)\Microsoft Visual Studio",
    ]:
        if os.path.isdir(base):
            for root, _, files in os.walk(base):
                if "dumpbin.exe" in files:
                    return os.path.join(root, "dumpbin.exe")
    return None


def parse_dumpbin_exports(text):
    """Extract exported symbol names from dumpbin /exports output."""
    symbols = set()
    in_exports = False
    for line in text.splitlines():
        if "ordinal hint" in line or "ordinal  hint" in line:
            in_exports = True
            continue
        if in_exports:
            line_str = line.strip()
            if not line_str:
                continue
            if line_str.startswith("Summary"):
                break
            # Match formats:
            # 1   F8 00043CC0 GiveFnptrsToDll = _GiveFnptrsToDll@8
            # 2    0 000097C0 ?AccelerateThink@CApacheHVR@@QAEXXZ = ...
            m = re.match(r"^\s*\d+\s+[0-9A-Fa-f\s]{1,8}\s+[0-9A-Fa-f]{8}\s+(\S+)", line)
            if m:
                sym = m.group(1)
                symbols.add(sym)
    return sorted(list(symbols))


def parse_nm_exports(text):
    """Extract exported symbols from Linux nm -D output."""
    symbols = set()
    for line in text.splitlines():
        line = line.strip()
        if not line:
            continue
        # Check for lines like:
        # 00043cc0 T GiveFnptrsToDll
        # or already filtered: GiveFnptrsToDll
        parts = line.split()
        if len(parts) >= 3 and parts[1] in ("T", "W", "t", "w"):
            sym = parts[2]
            if not sym.startswith("__"):
                symbols.add(sym)
        elif len(parts) == 1 and not parts[0].startswith("__"):
            symbols.add(parts[0])
    return sorted(list(symbols))


def parse_elf32_exports(filepath):
    """Pure-Python parser for 32-bit ELF dynamic symbol table (works on Windows/Linux)."""
    with open(filepath, "rb") as f:
        data = f.read()
    if len(data) < 52 or data[:4] != b"\x7fELF":
        raise ValueError(f"Not a valid ELF binary: {filepath}")
    if data[4] != 1:
        raise ValueError(f"Not a 32-bit ELF binary: {filepath}")

    e_shoff, e_flags, e_ehsize, e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = struct.unpack_from(
        "<IIHHHHHH", data, 32
    )

    sections = []
    for i in range(e_shnum):
        off = e_shoff + i * e_shentsize
        (
            sh_name,
            sh_type,
            sh_flags,
            sh_addr,
            sh_offset,
            sh_size,
            sh_link,
            sh_info,
            sh_addralign,
            sh_entsize,
        ) = struct.unpack_from("<10I", data, off)
        sections.append({
            "name_idx": sh_name,
            "type": sh_type,
            "flags": sh_flags,
            "offset": sh_offset,
            "size": sh_size,
            "link": sh_link,
            "entsize": sh_entsize,
        })

    # Read .shstrtab
    shstr_sec = sections[e_shstrndx]
    shstrtab = data[shstr_sec["offset"] : shstr_sec["offset"] + shstr_sec["size"]]
    for s in sections:
        end = shstrtab.find(b"\x00", s["name_idx"])
        s["name"] = shstrtab[s["name_idx"] : end].decode("ascii", errors="ignore")

    # Find .dynsym (type 11) and its linked .dynstr
    dynsym_sec = next((s for s in sections if s["type"] == 11), None)
    if not dynsym_sec:
        return []
    dynstr_sec = sections[dynsym_sec["link"]]
    dynstr = data[dynstr_sec["offset"] : dynstr_sec["offset"] + dynstr_sec["size"]]

    symbols = set()
    num_syms = dynsym_sec["size"] // dynsym_sec["entsize"]
    for i in range(num_syms):
        off = dynsym_sec["offset"] + i * dynsym_sec["entsize"]
        st_name, st_value, st_size, st_info, st_other, st_shndx = struct.unpack_from(
            "<IIIBBH", data, off
        )
        st_bind = st_info >> 4
        # Defined (st_shndx != 0) and in executable text section
        if st_shndx != 0 and st_shndx < len(sections):
            sec = sections[st_shndx]
            if (sec["flags"] & 4) != 0 and st_bind in (1, 2) and st_name != 0:
                end = dynstr.find(b"\x00", st_name)
                sym_name = dynstr[st_name:end].decode("ascii", errors="ignore")
                if sym_name and not sym_name.startswith("__"):
                    symbols.add(sym_name)

    return sorted(list(symbols))


def extract_symbols_from_binary(filepath, dumpbin_path=None):
    """Extract exported symbols from PE DLL or ELF SO binary."""
    ext = os.path.splitext(filepath)[1].lower()
    if ext == ".dll":
        db = dumpbin_path or find_dumpbin()
        if not db:
            raise RuntimeError("dumpbin.exe not found. Please provide --dumpbin or run from VS dev prompt.")
        out = subprocess.check_output([db, "/exports", filepath], text=True, stderr=subprocess.STDOUT)
        return parse_dumpbin_exports(out)
    elif ext in (".so", ".dylib") or not ext:
        # Check if nm is available
        nm = shutil.which("nm")
        if nm:
            try:
                out = subprocess.check_output([nm, "-D", filepath], text=True, stderr=subprocess.DEVNULL)
                syms = parse_nm_exports(out)
                if syms:
                    return syms
            except Exception:
                pass
        # Fallback to pure-Python ELF parser
        return parse_elf32_exports(filepath)
    else:
        raise ValueError(f"Unknown binary file format: {filepath}")


def load_symbols_file(filepath):
    """Load symbol list from text file (one symbol per line, ignore comments/blank)."""
    with open(filepath, "r", encoding="utf-8") as f:
        text = f.read()

    # If it's a raw dumpbin or nm output, parse it
    if "Section contains the following exports" in text:
        return parse_dumpbin_exports(text)
    elif " T " in text or " W " in text:
        return parse_nm_exports(text)

    symbols = []
    for line in text.splitlines():
        line = line.strip()
        if line and not line.startswith("#"):
            symbols.append(line)
    return sorted(list(set(symbols)))


def save_symbols_file(filepath, symbols):
    """Save symbol list to text file (one per line, sorted)."""
    os.makedirs(os.path.dirname(os.path.abspath(filepath)), exist_ok=True)
    with open(filepath, "w", encoding="utf-8") as f:
        for sym in sorted(symbols):
            f.write(f"{sym}\n")


def verify(golden_file, current_symbols):
    """Compare current symbols against golden baseline."""
    if not os.path.exists(golden_file):
        print(f"ERROR: Golden baseline file not found: {golden_file}", file=sys.stderr)
        return False

    golden_symbols = set(load_symbols_file(golden_file))
    current_set = set(current_symbols)

    missing = sorted(list(golden_symbols - current_set))
    new_symbols = sorted(list(current_set - golden_symbols))

    print(f"Comparing against golden: {golden_file}")
    print(f"  Golden symbols : {len(golden_symbols)}")
    print(f"  Current symbols: {len(current_set)}")

    if new_symbols:
        print(f"  WARNING: {len(new_symbols)} new exported symbol(s) detected (generally safe):")
        for sym in new_symbols[:10]:
            print(f"    + {sym}")
        if len(new_symbols) > 10:
            print(f"    ... and {len(new_symbols) - 10} more")

    if missing:
        print(f"  FAILED: {len(missing)} required exported symbol(s) MISSING!", file=sys.stderr)
        for sym in missing[:20]:
            print(f"    - {sym}", file=sys.stderr)
        if len(missing) > 20:
            print(f"    ... and {len(missing) - 20} more", file=sys.stderr)
        return False

    print("  PASSED: All required exported symbols are present.")
    return True


def main():
    parser = argparse.ArgumentParser(description="Exported symbol table verification (Layer 4)")
    parser.add_argument("--golden", required=True, help="Path to golden symbols text file")
    parser.add_argument("--binary", help="Path to compiled .dll or .so binary")
    parser.add_argument("--current", help="Path to text file with current symbols or raw dump")
    parser.add_argument("--dumpbin", help="Explicit path to dumpbin.exe")
    parser.add_argument("--update", action="store_true", help="Update golden file with current symbols")

    args = parser.parse_args()

    if not args.binary and not args.current:
        parser.error("Either --binary or --current must be provided.")

    if args.binary:
        symbols = extract_symbols_from_binary(args.binary, args.dumpbin)
    else:
        symbols = load_symbols_file(args.current)

    if args.update:
        save_symbols_file(args.golden, symbols)
        print(f"Updated golden baseline with {len(symbols)} symbols: {args.golden}")
        return 0

    success = verify(args.golden, symbols)
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
