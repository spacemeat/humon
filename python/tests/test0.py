import faulthandler
faulthandler.enable()
from pprint import pprint

from humon import humon

t = humon.Trove.fromString('@bug:ant{foo:bar}@baz:cat//cow\n//pig')
r = t.root
#print (f"\t\tdir(r): '{dir(r)}'")
#print (f"\t\trepr(r): '{repr(r)}'")
#print (f"\t\tstr(r): '{str(r)}'")
print ("before = dontchaknow")
snik = str(r)
print ("after = tryit")     # << !! This string is being NULLd
print (snik)
print ('after snik')
if r:
    print ("\t\tfoo done been gotten")
print (f"r.kind: {r.kind}")
#   print (f"r.keyToken: {r.keyToken}") # should return None
print (f"r.valueToken: {r.valueToken}")
print (f"r.address: {r.address}")
#print (f"r.parent == r1: {r.parent == r1}")    # false for now
print (f"r.tokenStream: {r.tokenStream}")
print (f"r.getAnnotations(key='baz'): {r.getAnnotations(key='baz')}")
print (f"r.getAnnotations(value='cat'): {r.getAnnotations(value='cat')}")
print (f"r.getComment(0): {r.getComment(0)}")
