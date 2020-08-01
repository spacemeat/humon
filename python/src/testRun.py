import faulthandler
faulthandler.enable()
from pprint import pprint

from humon import humon

t = humon.Trove('@bug:ant{foo:bar}@baz:cat//cow\n//pig')
print (f"\t\ttrove anno bug: {t.getTroveAnnotations(key='bug')}")
print (f"\t\ttrove anno with value 'ant': {t.getTroveAnnotations(value='ant')[0]}")
print (f"\t\ttrove comment: {t.getTroveComment(0)}")
k = t.getToken(1)
print (f"\t\ttoken 1: {k}")
r = t.root
print (f"\t\trepr(r): '{repr(r)}'")
print (f"\t\tstr(r): '{str(r)}'")
print (f"\t\tdir(r): '{dir(r)}'")
print (f"\t\tt.root: {t.root}")
print (t.root)
r1 = t.getNodeByIndex(0)
print (f"\t\tr1.kind: {str(r1.kind)}")
print (f"\t\tr1.address: {r1.address}")
print (f"\t\tr.kind: {str(r.kind)}")
print (f"\t\tWait for it")
print (f"\t\tr.address: {r.address}")
s = r.address
print ("bibbity")
ss = s
print ("bobbity")
print(f"type(ss): {type(ss)}")
print ("boo")
print(f"ss: {ss}")
print ("\t\tWait for it")
#print (r == r1) # false for now, but eventually must be true
kvn = r.getChild(0)
print ("\t\tgot child")
kvn = r.getChild("foo")
if kvn:
    print ("\t\tfoo child done been gotten")
print ("\t\tgot child")
print(f"\t\tdir(kvn): {dir(kvn)}")
print (f"kvn.kind: {str(kvn.kind)}")
print (f"kvn.keyToken: {kvn.keyToken}")
print (f"kvn.valueToken: {kvn.valueToken}")
print (f"kvn.address: {kvn.address}")
print (f"kvn.parent == r1: {kvn.parent == r1}")    # false for now
print (f"kvn.nestedValue: {kvn.nestedValue}")
print (f"kvn.getAnnotations(key='baz'): {r.getAnnotations(key='baz')}")
print (f"kvn.getAnnotations(value='cat'): {r.getAnnotations(value='cat')}")
#print (f"kvn.getComment(0): {r.getComment(-1)}")
s = r.getComment(-1)
print(s)
