import faulthandler
faulthandler.enable()

from humon import humon

# make the data structure
t = humon.Trove.fromString('{}//cow')
ts = t.getToken(0)
#print(ts)
# get the root of the structure
r = t.getNodeByIndex(0)
s = str(r.kind)
print (s)
