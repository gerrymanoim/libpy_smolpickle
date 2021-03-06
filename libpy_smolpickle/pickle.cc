#include <iostream>
#include <stack>
#include <string>
#include <unordered_set>

#include <libpy/autoclass.h>
#include <libpy/autofunction.h>
#include <libpy/automodule.h>
#include <libpy/to_object.h>

namespace libpy_smolpickle {

enum class protocols : int {
    LOWEST_PROTOCOL = 5,
    HIGHEST_PROTOCOL = 5,
    DEFAULT_PROTOCOL = 5,
};

enum class opcode : char {
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

// TODO - do we need this?
enum class batchsize : int {
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
        // TODO - is there a better way to do this
        long num = 0;
        for (std::size_t i = 0; i < n_to_read; i++) {
            num |= long(static_cast<unsigned char>(tape[tape_head + 1 + i])) << (8 * i);
        }
        // via https://github.com/jcrist/smolpickle/blob/3d2adcf7a024448e1756c4d6fb5173803e3ddbe8/smolpickle.c#L2010
        if (SIZEOF_LONG > 4 && n_to_read == 4) {
            num |= -(num & (1L << 31));
        }

        m_stack.push(py::to_object(num));
        return tape_head + n_to_read + 1;
    }

    int load_string(std::string_view tape, std::size_t tape_head, std::size_t n_to_read) {
        auto s = std::string(tape.substr(tape_head + 1, n_to_read + 1));  // c_str?
        m_stack.push(py::to_object(s));
        return tape_head + n_to_read + 1;
    }

    py::owned_ref<> loads(std::string_view tape) {
        for (std::size_t tape_head = 0; tape_head < tape.length(); ++tape_head) {
            opcode op = static_cast<opcode>(tape[tape_head]);
            switch (op) {
            case opcode::PROTO: {
                // advance it one to get the protocol
                // throw an error if it is too low
                tape_head += 1;
                int proto = tape[tape_head];
                if (static_cast<protocols>(proto) > protocols::HIGHEST_PROTOCOL) {
                    throw py::exception(PyExc_ValueError,
                                        "Unsupported pickle protocol",
                                        proto);
                }
            } break;
            case opcode::NONE:
                m_stack.push(py::none);
                break;
            case opcode::NEWTRUE:
                m_stack.push(py::to_object(true));
                break;
            case opcode::NEWFALSE:
                m_stack.push(py::to_object(false));
                break;
            case opcode::STOP: {

            } break;
            case opcode::BININT:
                tape_head = load_binint(tape, tape_head, 4);
                break;
            case opcode::BININT1:
                tape_head = load_binint(tape, tape_head, 1);
                break;
            case opcode::BININT2:
                tape_head = load_binint(tape, tape_head, 2);
                break;
            case opcode::FRAME:
                // not entirely sure I need frames
                // TODO - deal with this properly instead of skipping
                // https://www.python.org/dev/peps/pep-3154/#framing
                tape_head += 8;
                break;
            case opcode::MEMOIZE:
                m_memo.insert(m_stack.top());
                break;
            // todo I don't know that this is right
            case opcode::LONG1:
                tape_head = load_binint(tape, tape_head, 1);
                break;
            case opcode::LONG4:
                tape_head = load_binint(tape, tape_head, 4);
                break;
            case opcode::BINFLOAT: {
                float f = std::stof(tape.substr(tape_head + 1, 8).data());
                m_stack.push(py::to_object(f));
                tape_head += 8 + 1;
            } break;
            case opcode::SHORT_BINBYTES:
                tape_head = load_string(tape, tape_head, 1);
                break;
            case opcode::BINBYTES:
                tape_head = load_string(tape, tape_head, 4);
                break;
            case opcode::BINBYTES8:
                tape_head = load_string(tape, tape_head, 8);
                break;
            // end section I think is just wrong
            default:
                throw py::exception(PyExc_ValueError,
                                    "Unknown op code encountered",
                                    tape[tape_head]);
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
