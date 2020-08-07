#include <stack>
#include <string>
#include <unordered_set>

#include <libpy/autoclass.h>
#include <libpy/autofunction.h>
#include <libpy/automodule.h>
#include <libpy/to_object.h>

namespace libpy_smolpickle {

enum {
    LOWEST_PROTOCOL = 5,
    HIGHEST_PROTOCOL = 5,
    DEFAULT_PROTOCOL = 5,
};

enum opcode {
    MARK = '(',
    STOP = '.',
    POP = '0',
    POP_MARK = '1',
    BININT = 'J',
    BININT1 = 'K',
    BININT2 = 'M',
    NONE = 'N',
    BINUNICODE = 'X',
    APPEND = 'a',
    EMPTY_DICT = '}',
    APPENDS = 'e',
    BINGET = 'h',
    LONG_BINGET = 'j',
    EMPTY_LIST = ']',
    BINPUT = 'q',
    LONG_BINPUT = 'r',
    SETITEM = 's',
    TUPLE = 't',
    EMPTY_TUPLE = ')',
    SETITEMS = 'u',
    BINFLOAT = 'G',

    /* Protocol 2. */
    PROTO = '\x80',
    TUPLE1 = '\x85',
    TUPLE2 = '\x86',
    TUPLE3 = '\x87',
    NEWTRUE = '\x88',
    NEWFALSE = '\x89',
    LONG1 = '\x8a',
    LONG4 = '\x8b',

    /* Protocol 3 (Python 3.x) */
    BINBYTES = 'B',
    SHORT_BINBYTES = 'C',

    /* Protocol 4 */
    SHORT_BINUNICODE = '\x8c',
    BINUNICODE8 = '\x8d',
    BINBYTES8 = '\x8e',
    EMPTY_SET = '\x8f',
    ADDITEMS = '\x90',
    FROZENSET = '\x91',
    MEMOIZE = '\x94',
    FRAME = '\x95',

    /* Protocol 5 */
    BYTEARRAY8 = '\x96',
    NEXT_BUFFER = '\x97',
    READONLY_BUFFER = '\x98',

    /* Unsupported */
    DUP = '2',
    FLOAT = 'F',
    INT = 'I',
    LONG = 'L',
    REDUCE = 'R',
    STRING = 'S',
    BINSTRING = 'T',
    SHORT_BINSTRING = 'U',
    UNICODE = 'V',
    PERSID = 'P',
    BINPERSID = 'Q',
    GLOBAL = 'c',
    BUILD = 'b',
    DICT = 'd',
    LIST = 'l',
    OBJ = 'o',
    INST = 'i',
    PUT = 'p',
    GET = 'g',
    NEWOBJ = '\x81',
    EXT1 = '\x82',
    EXT2 = '\x83',
    EXT4 = '\x84',
    NEWOBJ_EX = '\x92',
    STACK_GLOBAL = '\x93',
};

enum {
    /* Number of elements save_list/dict/set writes out before
     * doing APPENDS/SETITEMS/ADDITEMS. */
    BATCHSIZE = 1000,
};

class picker {};

class unpickler {
private:
    std::stack<py::owned_ref<>> m_stack;
    std::unordered_set<py::object_map_key> m_memo;

public:
    // TODO - take real parameters
    unpickler() = default;

    int load_binint(std::string_view tape, std::size_t tape_head, std::size_t n_to_read) {
        long num = std::stol(tape.substr(tape_head + 1, n_to_read).c_str());
        m_stack.push(num);
        return tape_head + n_to_read + 1;
    }

    py::owned_ref<> loads(std::string_view tape) {
        for (std::size_t tape_head = 0; tape_head < tape.length(); ++tape_head) {
            opcode op = static_cast<opcode>(tape[tape_head]);
            switch (op) {
            case PROTO: {
                // advance it one to get the protocol
                // throw an error if it is too low
                tape_head += 1;
                int proto = tape[tape_head];
                if (proto > HIGHEST_PROTOCOL) {
                    throw py::exception(PyExc_ValueError,
                                        "Unsupported pickle protocol",
                                        proto);
                }
            } break;
            case NONE:
                m_stack.push(py::none);
                break;
            case BININT:
                tape_head = load_binint(tape, tape_head, 4);
                break;
            case BININT1:
                tape_head = load_binint(tape, tape_head, 1);
                break;
            case BININT2:
                tape_head = load_binint(tape, tape_head, 2);
                break;
            case STOP: {

            } break;
            default:
                throw py::exception(PyExc_ValueError, "Unknown op code encountered", tape[tape_head]);
            }
        }
        auto out = m_stack.top();
        m_stack.pop();
        return out;
    }

    // py::owned_ref<> load(const std::filesystem::path& filename) {}
};

LIBPY_AUTOMODULE(libpy_smolpickle, pickle, ({}))(py::borrowed_ref<> m) {
    py::autoclass<picker>(m, "Pickler").doc("Pickler").type();
    py::autoclass<unpickler>(m, "Unpickler")
        .new_<>()
        .doc("The Unpickler object TODO")
        .def<&unpickler::loads>("loads")
        .type();
    return false;
}
}  // namespace libpy_smolpickle
