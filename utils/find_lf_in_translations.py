#!/usr/bin/env python
# encoding: utf-8

import os
import os.path as p

"""
This tool detects illegal linebreaks in *.po (translation) files
which lead to bugs prior to the build 15 release. It is designed
to be used as makeprg inside of vim to easily jump to all locations
were line breaks are used, but it can also be used as a cmdline
tool to just print the files and line numbers.

The same can also be achieved by using make lang, but I had path
problems with it, so I hacked this little script.
"""

base_path = p.abspath(p.join(p.dirname(__file__),p.pardir))

def check_for_lf_in_translations(root_dname):
    for (dname, dnames, fnames) in os.walk(root_dname):
        for pfile in (f for f in fnames if p.splitext(f)[-1].lower() == '.po'):
            fname = p.relpath(p.join(dname,pfile))

            # open the .po file as binary to keep '\r\n' in tact
            for lidx,l in enumerate(open(fname, "rb")):
                if l.find(r'\r') != -1:
                    print "%s:%i: LF found!" % (fname, lidx+1)


if __name__ == '__main__':
    check_for_lf_in_translations(p.join(base_path, "po"))
