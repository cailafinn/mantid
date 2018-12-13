def sequoia(name):
    name = name.strip().upper()
    packs = ['B%s' % i for i in range(1, 37+1)] \
                + ['C%s' % i for i in range(1, 24+1)] \
                + ['C25T', 'C26T', 'C25B', 'C26B'] \
                + ['C%s' % i for i in range(27, 37+1)] \
                + ['D%s' % i for i in range(1, 37+1)]
    start_index = 38
    return start_index + packs.index(name)
