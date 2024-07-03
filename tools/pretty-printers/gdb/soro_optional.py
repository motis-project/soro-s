import gdb


class SoroOptionalPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val['val_'])


def print_soro_optional(val):
    type_str = str(val.type.strip_typedefs().unqualified())

    if type_str.startswith("soro::utls::optional"):
        return SoroOptionalPrinter(val)


gdb.pretty_printers.append(print_soro_optional)
