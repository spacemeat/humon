#!/usr/bin/python3

import os
from getVersion import getVersion

def printCache(cache):
    for src, srcv in cache.items():
        print(f"{src}: ")
        for frag, _ in srcv.items():
            print(f"    {frag}")

if __name__ == "__main__":
    version = getVersion()
    v3 = f"{version['major']}.{version['minor']}.{version['patch']}"

    readmeFile = open("README-preprocess.md", "rt")
    readmeLines = readmeFile.readlines()
    readmeFile.close()

    srcFileCache = {}

    readmeFile = open("README.md", "wt")

    for rmline in readmeLines:
        if '!!!HUMON_VERSION!!!' in rmline:
            rmline = rmline.replace('!!!HUMON_VERSION!!!', v3)

        if rmline.startswith("!!!"):
            kvplist = rmline[3:].split(';')
            src = ''
            frag = ''
            indent = 1
            for kvp in kvplist:
                k, v = kvp.split(':')
                k, v = (k.strip(), v.strip())
                if k == "src":
                    src = v.strip()
                elif k == "frag":
                    frag = v.strip()
                elif k == "indent":
                    indent += int(v.strip())
            
            if src != '':
                if src not in srcFileCache:
                    srcFile = open(src, "rt")
                    srcLines = srcFile.readlines()
                    srcFileCache[src] = {'*':[]}
                    thisFrag = ''
                    readingFrag = False
                    for idx, srcl in enumerate(srcLines):
                        idx = srcl.find('/*!!!eol*/')
                        if idx >= 0:
                            srcl = srcl[0:idx]
                        srcFileCache[src]['*'].append(srcl)
                        if not readingFrag:
                            if srcl.startswith("//!!!"):
                                thisFrag = srcl[5:].strip()
                                srcFileCache[src][thisFrag] = []
                                readingFrag = True
                        else:
                            if srcl.startswith("//!!!"):
                                thisFrag = ''
                                readingFrag = False
                            else:
                                srcFileCache[src][thisFrag].append(srcl)

                if frag != '':
                    if frag not in srcFileCache[src]:
                        printCache(srcFileCache)
                        raise RuntimeError(f"frag '{frag}' not found.'")
                    for srcl in srcFileCache[src][frag]:
                        if indent >= 0:
                            for i in range(0, indent):
                                print('    ', end='', file=readmeFile)
                            print(srcl.strip('\n'), file=readmeFile)
                        elif indent < 0:
                            print(srcl.strip('\n')[-4 * indent:], file=readmeFile)
                else:
                    for srcl in srcFileCache[src]['*']:
                        if indent > 0:
                            for i in range(0, indent):
                                print('    ', end='', file=readmeFile)
                            print(srcl.strip('\n'), file=readmeFile)
                        elif indent < 0:
                            print(srcl.strip('\n')[-4 * indent:], file=readmeFile)
        else:
            print(rmline.strip('\n'), file=readmeFile)

