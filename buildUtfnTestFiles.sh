echo "Building test files. Carefully examine each result to ensure it's"
echo "printing the way you want. These are the test cases for the printer."

function makePerms {
    echo "Scanning $1"
    samples/bin/hux $1 -px -my -cn -by -o $1.px.my.cn.by.hu
    samples/bin/hux $1 -px -my -cn -bn -o $1.px.my.cn.bn.hu
    samples/bin/hux $1 -px -my -ca -by -o $1.px.my.ca.by.hu
    samples/bin/hux $1 -px -my -ca -bn -o $1.px.my.ca.bn.hu
    samples/bin/hux $1 -px -mn -cn -by -o $1.px.mn.cn.by.hu
    samples/bin/hux $1 -px -mn -cn -bn -o $1.px.mn.cn.bn.hu
    samples/bin/hux $1 -px -mn -ca -by -o $1.px.mn.ca.by.hu
    samples/bin/hux $1 -px -mn -ca -bn -o $1.px.mn.ca.bn.hu
    samples/bin/hux $1 -pm -my -cn -by -o $1.pm.my.cn.by.hu
    samples/bin/hux $1 -pm -my -cn -bn -o $1.pm.my.cn.bn.hu
    samples/bin/hux $1 -pm -my -ca -by -o $1.pm.my.ca.by.hu
    samples/bin/hux $1 -pm -my -ca -bn -o $1.pm.my.ca.bn.hu
    samples/bin/hux $1 -pm -mn -cn -by -o $1.pm.mn.cn.by.hu
    samples/bin/hux $1 -pm -mn -cn -bn -o $1.pm.mn.cn.bn.hu
    samples/bin/hux $1 -pm -mn -ca -by -o $1.pm.mn.ca.by.hu
    samples/bin/hux $1 -pm -mn -ca -bn -o $1.pm.mn.ca.bn.hu
    samples/bin/hux $1 -pp -my -cn -by -o $1.pp.my.cn.by.hu
    samples/bin/hux $1 -pp -my -cn -bn -o $1.pp.my.cn.bn.hu
    samples/bin/hux $1 -pp -my -ca -by -o $1.pp.my.ca.by.hu
    samples/bin/hux $1 -pp -my -ca -bn -o $1.pp.my.ca.bn.hu
    samples/bin/hux $1 -pp -mn -cn -by -o $1.pp.mn.cn.by.hu
    samples/bin/hux $1 -pp -mn -cn -bn -o $1.pp.mn.cn.bn.hu
    samples/bin/hux $1 -pp -mn -ca -by -o $1.pp.mn.ca.by.hu
    samples/bin/hux $1 -pp -mn -ca -bn -o $1.pp.mn.ca.bn.hu
    echo ""
}

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
