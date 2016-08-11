#!/usr/bin/env python
# encoding: utf-8

from glob import glob
import os
import os.path as p
import re
import sys

###################
# inputs, outputs #
###################
# These files ar known to have rst comments; cpp files
# Meaning: (src_file, file_name_to_generate_rst)
cpp_pairs = (
    ('src/scripting/lua_root.cc', 'autogen_wl.rst'),
    ('src/scripting/lua_bases.cc', 'autogen_wl_bases.rst'),
    ('src/scripting/lua_editor.cc', 'autogen_wl_editor.rst'),
    ('src/scripting/lua_map.cc', 'autogen_wl_map.rst'),
    ('src/scripting/lua_game.cc', 'autogen_wl_game.rst'),
    ('src/scripting/lua_ui.cc', 'autogen_wl_ui.rst'),
    ('src/scripting/lua_globals.cc', 'autogen_globals.rst'),
)

# This directories are scanned without knowing which file
# has rst comments; scan for *.lua files
# Meaning: (src_dir, toc_name_to_place)
lua_dirs = (
    ('data/scripting', 'auxiliary'),
    ('data/scripting/editor', 'lua_world'),
    ('data/scripting/win_conditions', 'lua_world'),
    ('data/tribes', 'lua_tribes'),
    ('data/tribes/scripting/help', 'lua_tribes'),
    ('data/tribes/buildings/militarysites/atlanteans/castle', 'lua_tribes'),
)


def _find_basedir():
    """Walk upwards in the directory tree till we are in the base directory of
    Widelands.

    Return the base directory and the source/ directory.

    """
    curdir = p.abspath(p.dirname(__file__))
    source_dir = p.join(curdir, 'source')
    while not (p.exists(p.join(curdir, 'data/tribes')) and p.exists(p.join(curdir, 'data/world'))):
        curdir = p.abspath(p.join(curdir, p.pardir))

    return source_dir, curdir
source_dir, base_dir = _find_basedir()


def extract_rst_from_cpp(inname, outname=None):
    """
    Searches for /* RST comments in the given filename, strips the lines
    and prints them out on stdout or writes them to outname.
    """
    data = open(p.join(base_dir, inname), 'r').read()
    res = re.findall(r"\s*/\* RST\s(.*?)\*/", data, re.M | re.DOTALL)

    output = ''
    for r in res:
        r = r.expandtabs(4)
        output += r + '\n'

    if output.strip():
        out = sys.stdout
        if outname is not None:
            out = open(p.join(source_dir, outname), 'w')
        out.write(output)


def extract_rst_from_lua(directory, toc):
    """
    Searches for files with /* RST comments in the given directory, strips the lines
    and writes them to a file (each found file gets a new 'file.rst'). Returns a list
    with produced files for this directory.
    """
    # List to contain all created files
    rst_file_names = []
    # Find lua files and search for RST comments
    for filename in glob(p.join(base_dir, directory, '*.lua')):
        with open(p.join(base_dir, filename), 'r') as f:
            data = f.read()

        res = re.findall(r"-- RST\s(.*?)(?:^(?!--))", data, re.M | re.DOTALL)

        if res:
            # File with RST commet found
            outname = 'autogen_' + toc + '_%s.rst' % os.path.basename(
                os.path.splitext(filename)[0])
            rst_file_names.append(outname)
            outname = p.join(source_dir, outname)

            output = ''
            for r in res:
                r = re.subn(re.compile(r'^-- ?', re.M | re.DOTALL), '', r)[0]
                output += r + '\n'

            if output.strip():
                out = sys.stdout if not outname else open(outname, 'w')
                out.write(output)

    return rst_file_names


def replace_tocs(toc_rst_dict):
    """Open predefined toc files, add the produced filenames from
    extract_rst_from_lua() and store it as regular .rst file."""
    for toc_name, f_names in toc_rst_dict.items():

        # Open original toc file
        with open(p.join(source_dir, toc_name + '.rst.org'), 'r') as f:
            f_content = f.read()

        # Add the names to the content of original toc
        f_content = f_content.replace('REPLACE_ME',
                                      '\n'.join('   ' + name for name in f_names))

        # Save modified content as new file
        open(p.join(source_dir, 'autogen_toc_' +
                    toc_name + '.rst'), 'w').write(f_content)

if __name__ == '__main__':
    def main():
        for inf, outf in cpp_pairs:
            extract_rst_from_cpp(inf, outf)

        # Search the given dirs for rst-comments in every lua files
        # Store key:value as toc:list_of_files
        toc_fnames = {}
        for directory, toc in lua_dirs:
            rst_file_names = extract_rst_from_lua(directory, toc)
            if rst_file_names:
                if toc in toc_fnames:
                    # toc key exists, extend the value list
                    toc_fnames[toc].extend(rst_file_names)
                else:
                    # no toc found, create it and add value list
                    toc_fnames[toc] = rst_file_names

        # Replace the predefined tocs with all found files
        replace_tocs(toc_fnames)

    main()
