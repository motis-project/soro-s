class FractionPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(self.val['val_'])


def print_si_units(val):
    if not str(val.type.strip_typedefs().unqualified()).startswith(
            "soro::utls::fraction<"):
        return

    return FractionPrinter(val)


gdb.pretty_printers.append(print_si_units)
