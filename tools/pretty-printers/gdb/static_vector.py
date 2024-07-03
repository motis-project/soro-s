class StaticVectorPrinter:
    def __init__(self, val):
        self.val = val

    def children(self):
        for idx in range(self.val['end_']):
            yield '[' + str(idx) + ']', self.val['mem_']['_M_elems'][idx]

    def to_string(self):
        return str(self.val)


def print_static_vector(val):
    if str(val.type.strip_typedefs().unqualified()).startswith(
            "soro::utls::static_vector"):
        return StaticVectorPrinter(val)

    return


gdb.pretty_printers.append(print_static_vector)
