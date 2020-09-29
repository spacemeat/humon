class ChildNodeIterator:
    '''Iterates over a Humon node's children.'''
    def __init__(self, node):
        self.node = node
        self.idx = 0


    def __iter__(self):
        return self


    def __next__(self):
        '''Iterate.'''
        if self.idx < self.node.numChildren:
            res = self.node.getChild(self.idx)
            self.idx += 1
            return res
        raise StopIteration
