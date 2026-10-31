"""Execute compiled production policy helpers in pysmx, not a Python port.

The small adapter supplies standard SourceMod string/float host natives missing
from pysmx 0.4.0. It does not implement membership, trial or weapon policies.
This is VM-level coverage, not a TF2 engine simulation.
"""
from pathlib import Path
import ctypes
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools/python-libs'))
from smx.plugin import SourcePawnPlugin
from smx.sourcemod.natives import SourceModNatives
from smx.sourcemod.natives.base import native
from smx.sourcemod.printf import PrintfFormatter, atcprintf, formatfunc, render_int, render_string
from smx.reader import RTTITypeset
from smx.rtti import RTTI
from smx.definitions import RTTIControlByte


def decode_typeset(self, plugin, name, signature):
    """pysmx 0.4.0 treats an RTTI typeset offset as a type ID. Decode the
    metadata's counted type list instead; no production bytecode is changed.
    """
    super(RTTITypeset, self).__init__(plugin, name)
    self.signature = signature
    parser = plugin.rtti_parser_from_offset(signature)
    count = parser.decode_uint32()
    self.types = [parser.decode_new() for _ in range(count)]
    self.rtti = RTTI(plugin, type=RTTIControlByte.TYPESET, index=len(plugin.rtti_typesets))


RTTITypeset.__init__ = decode_typeset


class UnsignedFormatter(PrintfFormatter):
    @formatfunc('s', eats=1)
    def string(self, character, state):
        # pysmx's default reads immutable plugin data, losing strings changed
        # in runtime arrays/stack buffers. SourceMod formats the live heap.
        value = state.amx._getheapstring(state.params[state.arg])
        return render_string(value, state.flags, state.width, state.precision)

    @formatfunc('u', eats=1)
    def unsigned_integer(self, character, state):
        value = ctypes.c_uint32(state.amx._getheapcell(state.params[state.arg])).value
        return render_int(value, state.flags, state.width)


class TestHost(SourceModNatives):
    @native('cell')
    def float(self, value):
        return float(value)

    @native('string', 'string', 'writable_string')
    def SplitString(self, source, delimiter, destination):
        index = source.find(delimiter)
        if index < 0:
            return -1
        destination.write(source[:index], null_terminate=True)
        return index + len(delimiter)

    @native('string', 'cell')
    def StringToInt(self, source, base=10):
        match = re.match(r'\s*[+-]?\d+', source)
        return ctypes.c_int32(int(match.group(), base)).value if match else 0

    @native('cell', 'writable_string')
    def IntToString(self, value, destination):
        return destination.write(str(ctypes.c_int32(value).value), null_terminate=True)

    @native('writable_string', 'string', '...')
    def FormatEx(self, destination, format_string, *arguments):
        value = atcprintf(self.amx, format_string, arguments, formatter=UnsignedFormatter())
        value = value.encode('utf-8')[:max(0, destination.max_length - 1)].decode('utf-8', errors='ignore')
        return destination.write(value, null_terminate=True)

    @native('writable_string', 'string', 'string', 'bool')
    def ReplaceString(self, destination, search, replacement, case_sensitive=True):
        value = self.amx._getheapstring(destination.string_offs)
        if not search:
            return 0
        pattern = re.compile(re.escape(search), 0 if case_sensitive else re.IGNORECASE)
        value, count = pattern.subn(lambda match: replacement, value)
        value = value.encode('utf-8')[:max(0, destination.max_length - 1)].decode('utf-8', errors='ignore')
        destination.write(value, null_terminate=True)
        return count

    @native('string', '...')
    def PrintToServer(self, format_string, *arguments):
        print(atcprintf(self.amx, format_string, arguments, formatter=UnsignedFormatter()))

    @native('string', '...')
    def ThrowError(self, format_string, *arguments):
        raise AssertionError(atcprintf(self.amx, format_string, arguments, formatter=UnsignedFormatter()))

    @native('float', 'float')
    def __FLOAT_SUB__(self, left, right):
        return left - right

    @native('float', 'float')
    def __FLOAT_GT__(self, left, right):
        return left > right

    @native('float', 'float')
    def __FLOAT_LT__(self, left, right):
        return left < right

    @native('float', 'float')
    def __FLOAT_EQ__(self, left, right):
        return left == right

    @native('float', 'float')
    def __FLOAT_NE__(self, left, right):
        return left != right

    @native('float', 'float')
    def __FLOAT_LE__(self, left, right):
        return left <= right

    @native('float', 'float')
    def __FLOAT_GE__(self, left, right):
        return left >= right


def run(filename='core_tests.smx', minimum=500):
    with (ROOT / 'tests'/filename).open('rb') as stream:
        plugin = SourcePawnPlugin(stream, smsys_options={'natives_cls': TestHost})
    count = plugin.runtime.call_function_by_name('main')
    assert count > minimum, count
    print(f'\nExecuted {count} assertions against compiled production SourcePawn policy.')
    return count


if __name__ == '__main__':
    run()
    run('dependencies_tests.smx', 20)
    run('glow_tests.smx', 200)
    run('attributes_tests.smx', 10)
    run('buildings_tests.smx', 90)
    run('ammo_tests.smx', 15)

    run('options_tests.smx', 1000)
    run('celebrations_tests.smx', 10)
    run('airblast_tests.smx', 30)
    run('chat_tests.smx', 85)
    run('tooltip_tests.smx', 20)

    run('features_tests.smx', 380)

    run('economy_tests.smx', 200)
    run('ui_tests.smx', 40)
