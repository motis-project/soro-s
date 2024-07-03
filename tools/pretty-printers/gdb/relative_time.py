from datetime import timedelta


class RelativeTimePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return str(timedelta(seconds=int(self.val)))


def print_relative_time(val):
    if str(val.type.unqualified()).startswith("soro::relative_time") or str(val.type.unqualified()).startswith(
            "std::chrono::duration<unsigned int, std::ratio<1, 1> >"):
        # print('val', val)
        return RelativeTimePrinter(val)
    else:
        return


gdb.pretty_printers.append(print_relative_time)
