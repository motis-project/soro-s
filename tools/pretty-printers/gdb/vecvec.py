import re
import gdb.xmethod


class CistaVecVecPrinter:
    def __init__(self, val):
        self.val = val
        self.data = CistaVector(val['data_'])
        self.bucket_starts = CistaVector(val['bucket_starts_'])

    def children(self):
        command = f"{self.val['data_'].type}::value_type"
        value_type = gdb.lookup_type(command)
        data_begin = self.data.at(0).address

        for idx in range(len(self.bucket_starts) - 1):
            start, end = self.bucket_starts[idx], self.bucket_starts[idx + 1]
            span_command = f"get_my_span(({value_type}*){data_begin + start}, {end - start})"
            span = gdb.parse_and_eval(span_command)
            yield f"[{idx}]", span


def is_cista_vecvec(gdb_type):
    print("trying to look up")

    regex = re.compile("cista::basic_vecvec")
    type_str = str(gdb_type.strip_typedefs().unqualified())
    return regex.match(type_str) and "bucket" not in type_str


def my_pp_func(val):
    if is_cista_vecvec(val.type):
        return CistaVecVecPrinter(val)


class CistaVecVecWorker_operator_brackets(gdb.xmethod.XMethodWorker):
    def get_arg_types(self):
        return gdb.lookup_type('unsigned long int')

    def get_result_type(self, this, idx):
        return self(this, idx).type

    def __call__(self, this, idx):
        val = this.dereference()
        data = CistaVector(val['data_'])
        bucket_starts = CistaVector(val['bucket_starts_'])

        value_type = gdb.lookup_type(f"{val.type}::data_value_type").strip_typedefs()

        data_begin = data.at(0).address
        start, end = bucket_starts[idx], bucket_starts[idx + 1]

        span_command = f"get_my_span(({value_type}*){data_begin + start}, {end - start})"
        span = gdb.parse_and_eval(span_command)
        return span


class CistaVecVec_operator_brackets(gdb.xmethod.XMethod):
    def __init__(self):
        gdb.xmethod.XMethod.__init__(self, 'operator[]')

    def get_worker(self, method_name):
        if method_name == 'operator[]':
            return CistaVecVecWorker_operator_brackets()


class CistaVecVecMatcher(gdb.xmethod.XMethodMatcher):
    def __init__(self):
        gdb.xmethod.XMethodMatcher.__init__(self, "CistaVecVecMatcher")
        # List of methods 'managed' by this matcher
        self.methods = [CistaVecVec_operator_brackets()]

    def match(self, class_type, method_name):
        if not is_cista_vecvec(class_type):
            return None

        workers = []
        for method in self.methods:
            if method.enabled:
                worker = method.get_worker(method_name)
                if worker:
                    workers.append(worker)

        return workers


gdb.pretty_printers.append(my_pp_func)
gdb.xmethod.register_xmethod_matcher(None, CistaVecVecMatcher())
