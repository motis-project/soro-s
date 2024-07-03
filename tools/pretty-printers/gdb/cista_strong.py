class CistaStrongPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return self.val['v_'].format_string(raw=True)


def my_pp_func(val):
    type_str = str(val.type.strip_typedefs().unqualified())

    if type_str.startswith("cista::strong"):
        return CistaStrongPrinter(val)


gdb.pretty_printers.append(my_pp_func)
