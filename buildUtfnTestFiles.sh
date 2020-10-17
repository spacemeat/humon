#!/usr/bin/bash

echo "Building test files. Carefully examine each result to ensure it's"
echo "printing the way you want. These are the test cases for the printer."

function makeUtfns {
    # utf8.hu is the ultimate source; UTF-8 with no BOM. Ensure it stays like that when saving 
    # new versions of utf8.hu. The rest should be produced correctly here.
    echo "Generating UTF-n files"

    # make utf8bom.hu
    printf '\xef\xbb\xbf' > test/testFiles/utf8bom.hu
    cat test/testFiles/utf8.hu >> test/testFiles/utf8bom.hu

    # make utf16be.hu
    iconv -f UTF-8 -t UTF-16BE -o test/testFiles/utf16be.hu test/testFiles/utf8.hu

    # make utf16le.hu
    iconv -f UTF-8 -t UTF-16LE -o test/testFiles/utf16le.hu test/testFiles/utf8.hu

    # make utf32be.hu
    iconv -f UTF-8 -t UTF-32BE -o test/testFiles/utf32be.hu test/testFiles/utf8.hu

    # make utf32le.hu
    iconv -f UTF-8 -t UTF-32LE -o test/testFiles/utf32le.hu test/testFiles/utf8.hu

    # make utf16bebom.hu
    iconv -f UTF-8 -t UTF-16BE -o test/testFiles/utf16bebom.hu test/testFiles/utf8bom.hu

    # make utf16lebom.hu
    iconv -f UTF-8 -t UTF-16LE -o test/testFiles/utf16lebom.hu test/testFiles/utf8bom.hu

    # make utf32bebom.hu
    iconv -f UTF-8 -t UTF-32BE -o test/testFiles/utf32bebom.hu test/testFiles/utf8bom.hu

    # make utf32lebom.hu
    iconv -f UTF-8 -t UTF-32LE -o test/testFiles/utf32lebom.hu test/testFiles/utf8bom.hu
}

function makePerms {
    echo "Scanning $1"
    build/bin/hux $1 -pc -my -cn -by -o $1.pc.my.cn.by.hu
    build/bin/hux $1 -pc -my -cn -bn -o $1.pc.my.cn.bn.hu
    build/bin/hux $1 -pc -my -ca -by -o $1.pc.my.ca.by.hu
    build/bin/hux $1 -pc -my -ca -bn -o $1.pc.my.ca.bn.hu
    build/bin/hux $1 -pc -mn -cn -by -o $1.pc.mn.cn.by.hu
    build/bin/hux $1 -pc -mn -cn -bn -o $1.pc.mn.cn.bn.hu
    build/bin/hux $1 -pc -mn -ca -by -o $1.pc.mn.ca.by.hu
    build/bin/hux $1 -pc -mn -ca -bn -o $1.pc.mn.ca.bn.hu
    build/bin/hux $1 -pm -my -cn -by -o $1.pm.my.cn.by.hu
    build/bin/hux $1 -pm -my -cn -bn -o $1.pm.my.cn.bn.hu
    build/bin/hux $1 -pm -my -ca -by -o $1.pm.my.ca.by.hu
    build/bin/hux $1 -pm -my -ca -bn -o $1.pm.my.ca.bn.hu
    build/bin/hux $1 -pm -mn -cn -by -o $1.pm.mn.cn.by.hu
    build/bin/hux $1 -pm -mn -cn -bn -o $1.pm.mn.cn.bn.hu
    build/bin/hux $1 -pm -mn -ca -by -o $1.pm.mn.ca.by.hu
    build/bin/hux $1 -pm -mn -ca -bn -o $1.pm.mn.ca.bn.hu
    build/bin/hux $1 -pp -my -cn -by -o $1.pp.my.cn.by.hu
    build/bin/hux $1 -pp -my -cn -bn -o $1.pp.my.cn.bn.hu
    build/bin/hux $1 -pp -my -ca -by -o $1.pp.my.ca.by.hu
    build/bin/hux $1 -pp -my -ca -bn -o $1.pp.my.ca.bn.hu
    build/bin/hux $1 -pp -mn -cn -by -o $1.pp.mn.cn.by.hu
    build/bin/hux $1 -pp -mn -cn -bn -o $1.pp.mn.cn.bn.hu
    build/bin/hux $1 -pp -mn -ca -by -o $1.pp.mn.ca.by.hu
    build/bin/hux $1 -pp -mn -ca -bn -o $1.pp.mn.ca.bn.hu
    echo ""
}

makeUtfns

makePerms test/testFiles/comments.hu
makePerms test/testFiles/commentsCstyle.hu
makePerms test/testFiles/quothTheHumon.hu
makePerms test/testFiles/utf8.hu
makePerms test/testFiles/utf8bom.hu
makePerms test/testFiles/utf16be.hu
makePerms test/testFiles/utf16bebom.hu
makePerms test/testFiles/utf16le.hu
makePerms test/testFiles/utf16lebom.hu
makePerms test/testFiles/utf32be.hu
makePerms test/testFiles/utf32bebom.hu
makePerms test/testFiles/utf32le.hu
makePerms test/testFiles/utf32lebom.hu
