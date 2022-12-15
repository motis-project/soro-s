class SerializablePrinter:
    def __init__(self, val):
        self.access_ = None

        for field in val.type.fields():
            if str(field.type.strip_typedefs().unqualified()).startswith(
                    "soro::utls::serializable<"):
                self.access_ = val.cast(field.type)['access_']

    def children(self):
        for field in self.access_.dereference().type.fields():
            yield field.name, self.access_.dereference()[field.name].cast(
                field.type)


def match_infrastructure(val):
    if not str(val.type.strip_typedefs().unqualified()).startswith(
            "soro::infra::infrastructure") or str(
        val.type.strip_typedefs().unqualified()).startswith(
        "soro::infra::infrastructure_t"):
        return

    return SerializablePrinter(val)


def match_timetable(val):
    if not str(val.type.strip_typedefs().unqualified()).startswith(
            "soro::tt::timetable"):
        return

    return SerializablePrinter(val)


gdb.pretty_printers.append(match_infrastructure)
gdb.pretty_printers.append(match_timetable)
