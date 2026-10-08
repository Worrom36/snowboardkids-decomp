import struct
import unittest

from match_rom_functions import (Function, assembly_splits, code_layout,
                                 infer_boundaries, match_functions, normalized, preserve_c_splits)

RAM = [(0x80000400, 0x80160000)]


def body(*words):
    return struct.pack(f'>{len(words)}I', *words)


def function(address, data, name='test'):
    return Function(name, address - 0x7ffff400, address, len(data), name, data)


def leaf(value):
    return body(0x24020000 | value, 0x24420001, 0x03e00008, 0)


class RegionMatcherTests(unittest.TestCase):
    def match(self, us, jp):
        return match_functions(us, jp, RAM, RAM)[0]

    def test_exact_shift(self):
        us = [function(0x80001000, leaf(2))]
        jp = [function(0x80002000, leaf(2))]
        self.assertEqual(self.match(us, jp), {0: 0})

    def test_normalized_data_and_calls(self):
        a = body(0x3c04800d, 0x24848000, 0x0c002000, 0, 0x03e00008, 0)
        b = body(0x3c04800e, 0x24848100, 0x0c002010, 0, 0x03e00008, 0)
        self.assertEqual(self.match([function(0x80001000, a)], [function(0x80002000, b)]), {0: 0})

    def test_meaningful_immediates_preserved(self):
        self.assertEqual(self.match([function(0x80001000, leaf(2))],
                                    [function(0x80002000, leaf(3))]), {})

    def test_register_change_preserved(self):
        a = body(0x3c04800d, 0x24848000, 0x03e00008, 0)
        b = body(0x3c05800d, 0x24a58000, 0x03e00008, 0)
        self.assertEqual(self.match([function(0x80001000, a)], [function(0x80002000, b)]), {})

    def test_non_ram_constant_preserved(self):
        a = body(0x3c041234, 0x24845678, 0x03e00008, 0)
        b = body(0x3c041234, 0x24845679, 0x03e00008, 0)
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])

    def test_clobber_stops_address_pair(self):
        a = body(0x3c04800d, 0x00852021, 0x24848000, 0x03e00008, 0)
        b = body(0x3c04800d, 0x00852021, 0x24848004, 0x03e00008, 0)
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])

    def test_control_flow_stops_address_pair(self):
        a = body(0x3c04800d, 0x10000001, 0, 0x24848000, 0x03e00008, 0)
        b = body(0x3c04800d, 0x10000001, 0, 0x24848004, 0x03e00008, 0)
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])

    def test_branch_target_stops_address_pair(self):
        a = body(0x3c04800d, 0x8c828000, 0x1440fffe, 0, 0x03e00008, 0)
        b = body(0x3c04800e, 0x8c828100, 0x1440fffe, 0, 0x03e00008, 0)
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])

    def test_scalar_use_of_address_high_half_is_not_masked(self):
        a = body(0x3c04800d, 0xafa40000, 0x8c828000, 0x03e00008, 0)
        b = body(0x3c04800e, 0xafa40000, 0x8c828100, 0x03e00008, 0)
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])

    def test_internal_jump_offsets_preserved(self):
        a = body(0x08000402, 0, 0x03e00008, 0)
        b = body(0x08000802, 0, 0x03e00008, 0)
        c = body(0x08000803, 0, 0x03e00008, 0)
        self.assertEqual(normalized(a, 0x80001000, RAM)[0], normalized(b, 0x80002000, RAM)[0])
        self.assertNotEqual(normalized(a, 0x80001000, RAM)[0], normalized(c, 0x80002000, RAM)[0])

    def test_inserted_function_and_deleted_function(self):
        us = [function(0x80001000 + i*16, leaf(x)) for i, x in enumerate([1, 2, 3, 4])]
        jp = [function(0x80002000 + i*16, leaf(x)) for i, x in enumerate([1, 7, 3, 4])]
        self.assertEqual(self.match(us, jp), {0: 0, 2: 2, 3: 3})

    def test_duplicate_stubs_need_neighbors(self):
        stub = body(0x03e00008, 0)
        us = [function(0x80001000, stub)]
        jp = [function(0x80002000, stub), function(0x80002010, stub)]
        result = match_functions(us, jp, RAM, RAM)
        self.assertEqual(result[0], {})
        self.assertEqual(result[1][0], [0, 1])
        us = [function(0x80001000+i*16, data) for i, data in enumerate([leaf(1), stub, leaf(2)])]
        jp = [function(0x80002000+i*16, data) for i, data in enumerate([leaf(1), stub, leaf(2), stub])]
        self.assertEqual(self.match(us, jp), {0: 0, 1: 1, 2: 2})

    def test_order_conflicts_are_reported(self):
        us = [function(0x80001000+i*16, leaf(x)) for i, x in enumerate([1, 2])]
        jp = [function(0x80002000+i*16, leaf(x)) for i, x in enumerate([2, 1])]
        result = match_functions(us, jp, RAM, RAM)
        self.assertEqual(result[0], {})
        self.assertEqual(result[3], {0, 1})

    def test_mapped_call_conflict(self):
        us = [function(0x80001000, body(0x0c000404, 0, 0x03e00008, 0)),
              function(0x80001010, leaf(1)), function(0x80001020, leaf(2))]
        jp = [function(0x80002000, body(0x0c000808, 0, 0x03e00008, 0)),
              function(0x80002010, leaf(1)), function(0x80002020, leaf(2))]
        self.assertNotIn(0, self.match(us, jp))

    def test_repeated_pointer_identity_preserved(self):
        a = body(0x3c04800d, 0x24848000, 0x3c05800d, 0x24a58000, 0x03e00008, 0)
        b = body(0x3c04800e, 0x24848000, 0x3c05800e, 0x24a58004, 0x03e00008, 0)
        self.assertEqual(self.match([function(0x80001000, a)], [function(0x80002000, b)]), {})

    def test_padding_not_treated_as_body(self):
        a = leaf(1)
        self.assertEqual(self.match([function(0x80001000, a)], [function(0x80002000, a+body(0))]), {})

    def test_layout_excludes_microcode_and_data(self):
        config = {'segments': [dict(type='code', start=0x1000, vram=0x80000400,
                                   subsegments=[[0x1000, 'asm', 'entry']]),
                              dict(type='code', start=0x1050, vram=0x80000450,
                                   subsegments=[[0x1050, 'c', 'main'],
                                                dict(start=0x2000, type='textbin', name='rsp'),
                                                [0x2100, '.data', 'main']])]}
        self.assertEqual([(a,b) for a,b,_,_ in code_layout(config)], [(0x1000,0x1050),(0x1050,0x2000)])

    def test_boundary_requires_entry_and_corroboration(self):
        us = [function(0x80000450+i*16, leaf(i), 'file') for i in range(3)]
        jp = [function(0x80000550+i*16, leaf(i), 'file') for i in range(3)]
        ranges = [(0x1050,0x1080,'file',0x7ffff400)]
        self.assertEqual(infer_boundaries(us,jp,ranges,{0:0})[0]['status'], 'approximate')
        self.assertEqual(infer_boundaries(us,jp,ranges,{0:0,1:1})[0]['status'], 'supported')
        missing = infer_boundaries(us,jp,ranges,{1:1,2:2})[0]
        self.assertEqual(missing['status'], 'approximate')
        self.assertIsNone(missing['jp_start'])

    def test_splits_do_not_assign_unknown_tail_to_named_file(self):
        boundaries = [dict(status='supported', jp_start=0x1050, file='known'),
                      dict(status='approximate', jp_start=None, file='unknown'),
                      dict(status='supported', jp_start=0x1080, file='next')]
        functions = [dict(file='known', status='matched', jp_rom=0x1050, size=0x14),
                     dict(file='next', status='matched', jp_rom=0x1080, size=0x10)]
        jp = [function(0x80000450, leaf(1)), function(0x80000468, leaf(2)),
              function(0x80000480, leaf(3))]
        splits = assembly_splits(boundaries, functions, jp, 0x1090)
        self.assertEqual([(s['start'], s['name']) for s in splits],
                         [(0x1050, 'known'), (0x1068, 'unresolved/001068'), (0x1080, 'next')])

    def test_split_export_preserves_integrated_c(self):
        config = {'segments': [dict(name='main', subsegments=[
            [0x1050, 'asm', 'prefix'], [0x1080, 'c', 'matched'],
            [0x1100, 'asm', 'suffix'], [0x1200, 'databin', 'data']])]}
        proposals = [dict(start=0x1050, name='whole_file', reason='supported-start')]
        result = preserve_c_splits(proposals, config)
        self.assertEqual([(s['start'], s['type'], s['name']) for s in result],
                         [(0x1050, 'asm', 'prefix'), (0x1080, 'c', 'matched'), (0x1100, 'asm', 'suffix')])


if __name__ == '__main__':
    unittest.main()
