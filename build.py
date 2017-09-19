#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# ---------------------------------------------------------------------
# Program: build.py
# Author:  Mauricio Caceres Bravo <mauricio.caceres.bravo@gmail.com>
# Created: Sun Jun 18 15:18:20 EDT 2017
# Updated: Sun Jul 30 20:26:15 EDT 2017
# Purpose: Main build file for qsort (copies contents into ./build and
#          puts a .zip file in ./releases)

from os import makedirs, path, linesep, chdir, system, remove, rename
from shutil import copy2, which, rmtree
from sys import platform
from tempfile import gettempdir
from zipfile import ZipFile
from re import search

import argparse
parser = argparse.ArgumentParser()
parser.add_argument('--stata',
                    nargs    = 1,
                    type     = str,
                    metavar  = 'STATA',
                    default  = None,
                    required = False,
                    help     = "Path to stata executable")
parser.add_argument('--stata-args',
                    nargs    = 1,
                    type     = str,
                    metavar  = 'STATA_ARGS',
                    default  = None,
                    required = False,
                    help     = "Arguments to pass to Stata executable")
parser.add_argument('--clean',
                    dest     = 'clean',
                    action   = 'store_true',
                    help     = "Clean build",
                    required = False)
parser.add_argument('--replace',
                    dest     = 'replace',
                    action   = 'store_true',
                    help     = "Replace build",
                    required = False)
parser.add_argument('--test',
                    dest     = 'test',
                    action   = 'store_true',
                    help     = "Run tests",
                    required = False)
parser.add_argument('--windows',
                    dest     = 'windows',
                    action   = 'store_true',
                    help     = "Compile for Windows from Unix environment.",
                    required = False)
args = vars(parser.parse_args())

def makedirs_safe(directory):
    try:
        makedirs(directory)
        return directory
    except OSError:
        if not path.isdir(directory):
            raise

qsort_ssc = [
    "qsort.ado",
    "qsort.sthlp"
]

qsort_zip = [
    "changelog.md",
    "qsort.pkg",
    "stata.toc"
] + qsort_ssc

qsort_build = qsort_zip + [
    "qsort_tests.do"
]

# Remove buld
# -----------

if args['clean']:
    print("Removing build files")
    for bfile in qsort_build:
        try:
            remove(path.join("build", bfile))
            print("\tdeleted " + bfile)
        except:
            try:
                remove(path.join("build", "qsort", bfile))
                print("\tdeleted " + bfile)
            except:
                print("\t" + bfile + " not found")

    rc = system("make clean")
    exit(0)

makedirs_safe(path.join("build", "qsort"))
makedirs_safe("releases")

# Stata executable
# ----------------

# I don't have stata on my global path, so to make the script portable
# I make it look for my local executable when Stata is not found.
if args['stata'] is not None:
    statadir = path.abspath(".")
    stataexe = args['stata'][0]
    statargs = "-b do" if args['stata_args'] is None else args['stata_args'][0]
    statado  = '"{0}" {1}'.format(stataexe, statargs)
elif which("stata") is None:
    statadir = path.expanduser("~/.local/stata13")
    stataexe = path.join(statadir, "stata")
    statargs = "-b do" if args['stata_args'] is None else args['stata_args']
    statado  = '"{0}" {1}'.format(stataexe, statargs)
else:
    statadir = path.abspath(".")
    stataexe = 'stata'
    statargs = "-b do" if args['stata_args'] is None else args['stata_args']
    statado  = '"{0}" {1}'.format(stataexe, statargs)

# Temporary files
# ---------------

maindir   = path.dirname(path.realpath(__file__))
tmpdir    = gettempdir()
tmpupdate = path.join(tmpdir, ".update_qsort.do")

# Compile plugin files
# --------------------

if platform in ["linux", "linux2", "win32", "cygwin", "darwin"]:
    print("Trying to compile plugins for -qsort-")
    rc = system("make")
    print("Success!" if rc == 0 else "Failed.")
    if args['windows']:
        rc = system("make EXECUTION=windows clean")
        rc = system("make EXECUTION=windows spooky")
        rc = system("make EXECUTION=windows")
else:
    print("Don't know platform '{0}'; compile manually.".format(platform))
    exit(198)

print("")

# Get unit test files
# -------------------

testfile = open(path.join("src", "test", "qsort_tests.do")).readlines()
files    = [path.join("src", "test", "qsort_checks.do")]

with open(path.join("build", "qsort_tests.do"), 'w') as outfile:
    outfile.writelines(testfile[:-4])

with open(path.join("build", "qsort_tests.do"), 'a') as outfile:
    for fname in files:
        with open(fname) as infile:
            outfile.write(infile.read())

    outfile.writelines(testfile[-5:])

# Copy files to ./build
# ---------------------

gdir = path.join("build", "qsort")
copy2("changelog.md", gdir)
copy2(path.join("src", "qsort.pkg"), gdir)
copy2(path.join("src", "stata.toc"), gdir)
copy2(path.join("doc", "qsort.sthlp"), gdir)
copy2(path.join("src", "ado", "qsort.ado"), gdir)

# Copy files to .zip folder in ./releases
# ---------------------------------------

# Get stata version
with open(path.join("src", "ado", "qsort.ado"), 'r') as f:
    line    = f.readline()
    version = search('(\d+\.?)+', line).group(0)

plugins = ["qsort_unix.plugin",
           "qsort_unix_multi.plugin",
           "qsort_windows.plugin",
           "qsort_macosx.plugin"]
plugbak = plugins.copy()
for plug in plugbak:
    if not path.isfile(path.join("build", plug)):
        alt = path.join("lib", "plugin", plug)
        if path.isfile(alt):
            copy2(alt, "build")
        else:
            print("Could not find '{0}'".format(plug))

chdir("build")
print("Compressing build files for qsort-{0}".format(version))
if rc == 0:
    qsort_anyplug = False
    for plug in plugbak:
        if path.isfile(plug):
            qsort_anyplug = True
            rename(path.join(plug), path.join("qsort", plug))
        else:
            plugins.remove(plug)
            print("\t'{0}' not found; skipping.".format(plug))

    if not qsort_anyplug:
        print("WARNING: Could not find plugins despite build exit with 0 status.")
        exit(-1)

    qsort_zip += plugins
else:
    print("WARNING: Failed to build plugins. Will exit.")
    exit(-1)

outzip = path.join(maindir, "releases", "qsort-{0}.zip".format(version))
with ZipFile(outzip, 'w') as zf:
    for zfile in qsort_zip:
        zf.write(path.join("qsort", zfile))
        print("\t" + path.join("qsort", zfile))
        rename(path.join("qsort", zfile), zfile)

chdir(maindir)
copy2(outzip, path.join("releases", "qsort-latest.zip"))
rmtree(path.join("build", "qsort"))

# Copy files to send to SSC
# -------------------------

print("")
print("Compressing build files for qsort-ssc.zip")
if rc == 0:
    qsort_ssc += plugins
else:
    print("WARNING: Failed to build plugins. Will exit.")
    exit(-1)

chdir("build")
outzip = path.join(maindir, "releases", "qsort-ssc.zip")
with ZipFile(outzip, 'w') as zf:
    for zfile in qsort_ssc:
        zf.write(zfile)
        print("\t" + zfile)

# Replace package in ~/ado/plus
# -----------------------------

chdir(maindir)
if args["replace"]:
    if which(stataexe):
        with open(tmpupdate, 'w') as f:
            f.write("global builddir {0}".format(path.join(maindir, "build")))
            f.write(linesep)
            f.write("cap net uninstall qsort")
            f.write(linesep)
            f.write("net install qsort, from($builddir)")
            f.write(linesep)

        chdir(statadir)
        system(statado + " " + tmpupdate)
        print(linesep + "Replaced qsort in ~/ado/plus")
        chdir(maindir)
    else:
        print("Could not find Stata executable '{0}'.".format(stataexe))
        exit(-1)

# Run tests
# ---------

if args['test']:
    print("Running tests (see build/qsort_tests.log for output)")
    chdir("build")
    system(statado + " qsort_tests.do")
    chdir(maindir)
