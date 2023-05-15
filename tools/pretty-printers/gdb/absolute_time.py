from datetime import datetime


class AbsoluteTimePrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return datetime.utcfromtimestamp(self.val['__d']['__r']).strftime(
            "%d.%m.%Y - %H:%M:%S")


def print_absolute_time(val):
    if not str(val.type.unqualified()).startswith(
            "soro::absolute_time"):
        return

    return AbsoluteTimePrinter(val)


gdb.pretty_printers.append(print_absolute_time)
