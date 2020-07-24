import humon

hu = humon.fromString("{a: b c: {d: e f: g} h: [i j k]}")

valA = hu / 'a'
valD = hu / 'c' / 'd'

valC = hu / 'c'
valF = valC / 'f'

valH = hu / 'h'
valI = valH / 0

