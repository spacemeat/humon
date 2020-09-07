import faulthandler
faulthandler.enable()
from pprint import pprint

from .context import humon

t = humon.Trove('@bug:ant{foo:bar}@{baz:cat moo:cat}//cow\n//pig')
print (f"trove token stream: {t.tokenStream}")
print (f"trove anno bug: {t.getTroveAnnotations(key='bug')}")
print (f"trove anno with value 'ant': {t.getTroveAnnotations(value='ant')[0]}")
print (f"trove comment: {t.getTroveComment(0)}")
k = t.getToken(1)
print (f"token 1: {k}")
r = t.root
print (f"repr(r): {repr(r)}")
print (f"str(r): {str(r)}")
print (f"r.tokenStream: {r.tokenStream}")
print (f"t.root.address: {t.root.address}")
r1 = t.getNodeByIndex(0)
print (f"r1.kind: {str(r1.kind)}")
print (f"r1.address: {r1.address}")
print (f"r.kind: {str(r.kind)}")
print (f"Wait for it")
print (f"r.address: {r.address}")
s = r.address
print ("bibbity")
ss = s
print ("bobbity")
print(f"type(ss): {type(ss)}")
print ("boo")
print(f"ss: {ss}")
print ("Wait for it")
print (f"r == r1: {r == r1}") # false for now, but eventually must be true
kvn = r.getChild(0)
print ("got child")
kvn = r.getChild("foo")
if kvn:
    print ("foo child done been gotten")
print ("got child")
print (f"kvn.kind: {str(kvn.kind)}")
print (f"kvn.keyToken: {kvn.keyToken}")
print (f"kvn.valueToken: {kvn.valueToken}")
print (f"kvn.address: {kvn.address}")
print (f"kvn.parent == r1: {kvn.parent == r1}")    # false for now
print (f"kvn.tokenStream: {kvn.tokenStream}")
print (f"kvn.getAnnotations(key='baz'): {r.getAnnotations(key='baz')}")
print (f"kvn.getAnnotations(value='cat'): {[str(k) for k in r.getAnnotations(value='cat')]}")
print (f"kvn.getComment(0): {r.getComment(0)}")
print (f"kvn.getComment(1): {r.getComment(1)}")
print (f"kvn.getComment(1) == None: {r.getComment(1) == None}")

if (r.getChild(2)):
    print ("has child 2")
else:
    print ("has no child 2")

if (r.getComment(2)):
    print ("has comment 2")
else:
    print ("has no comment 2")

tt = humon.Trove("{foo}")
if tt:
    print ("Bogus trove is good, which is bad")
else:
    print ("Bogus trove is bogus, which is good and not bad")