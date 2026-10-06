import unittest

from tools.animation_assets import pack_animation
from tools.format_shift_fixture import signature
from tools import test_shift_animations as animation_tests
from tools import test_shift_sprites as sprite_tests
from tools import test_shift_tilemaps as tilemap_tests


class SemanticFixtureTests(unittest.TestCase):
    def test_wrong_shared_animation_target_changes_signature(self):
        manifest = animation_tests.AnimationRelocationTests().manifest()
        data = pack_animation(manifest)
        wrong = bytearray(data)
        wrong[4:6] = wrong[0:2]
        spec = dict(format=manifest['format'], part_count=manifest['part_count'])
        self.assertNotEqual(signature(data, spec), signature(wrong, spec))

    def test_wrong_sprite_palette_changes_signature(self):
        manifest, data = sprite_tests.SpriteRelocationTests().fixture()
        wrong = bytearray(data)
        wrong[12:14] = (1).to_bytes(2, 'big')
        spec = dict(format='sprite_table', ci8=[])
        self.assertNotEqual(signature(data, spec), signature(wrong, spec))

    def test_wrong_tile_reference_changes_signature(self):
        manifest, data = tilemap_tests.TilemapRelocationTests().fixture()
        wrong = bytearray(data)
        wrong[28:30] = (2).to_bytes(2, 'big')
        spec = dict(format='tilemap', image_format='ci4')
        self.assertNotEqual(signature(data, spec), signature(wrong, spec))


if __name__ == '__main__':
    unittest.main()
