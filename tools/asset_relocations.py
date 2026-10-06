"""Object-identity layout and explicit packed-field relocation.

Offsets describe the original matching layout. IDs remain stable when objects
are reordered or resized. This module never infers pointers from numeric values.
"""
from dataclasses import dataclass


@dataclass(frozen=True)
class Object:
    id: str
    data: bytes
    offset: int
    alignment: int = 1
    alias: str | None = None


@dataclass(frozen=True)
class Reference:
    source: str
    field: int
    target: str
    kind: str = 'relative'
    addend: int = 0
    width: int = 4
    unit: int = 1
    segment: int = 0
    base: str | None = None


def relocate(asset, objects, references, *, mode='matching', stage='decoded', address=0):
    """Return (encoded objects, object offsets) after validating every fixup."""
    def fail(message):
        raise ValueError(f'{asset}: {message}')
    if mode not in ('matching', 'shiftable') or stage not in ('decoded', 'rom'):
        fail('unsupported layout mode or stage')
    by_id, offsets, cursor = {}, {}, 0
    for obj in objects:
        if not obj.id or obj.id in by_id:
            fail(f'duplicate or empty object identity {obj.id!r}')
        if obj.alignment <= 0 or obj.alignment & (obj.alignment-1):
            fail(f'{obj.id}: alignment must be a positive power of two')
        if obj.offset < 0:
            fail(f'{obj.id}: negative matching offset')
        by_id[obj.id] = obj
        if obj.alias is not None:
            if obj.data:
                fail(f'{obj.id}: alias must not allocate duplicate storage')
            continue
        offset = obj.offset if mode == 'matching' else (cursor+obj.alignment-1) & -obj.alignment
        if offset % obj.alignment:
            fail(f'{obj.id}: offset violates alignment {obj.alignment}')
        if offset < cursor:
            fail(f'{obj.id}: object overlaps preceding storage')
        offsets[obj.id] = offset
        cursor = offset + len(obj.data)
    resolving = set()
    def resolve(identity):
        if identity not in by_id:
            fail(f'unresolved object {identity!r}; cross-asset references require an explicit link stage')
        obj = by_id[identity]
        if obj.alias is None:
            return obj
        if identity in resolving:
            fail(f'{identity}: cyclic alias')
        resolving.add(identity)
        canonical = resolve(obj.alias)
        resolving.remove(identity)
        offsets[identity] = offsets[canonical.id]
        if mode == 'matching' and obj.offset != offsets[identity]:
            fail(f'{identity}: alias matching offset differs from shared target')
        return canonical
    for identity in by_id:
        resolve(identity)
    result = bytearray(cursor)
    for obj in objects:
        if obj.alias is None:
            start = offsets[obj.id]
            result[start:start+len(obj.data)] = obj.data
    fields = set()
    for ref in references:
        label = f'{ref.source}+0x{ref.field:X} -> {ref.target}'
        try:
            source, target = resolve(ref.source), resolve(ref.target)
        except ValueError as exc:
            fail(f'{label}: {exc}')
        if ref.width not in (1, 2, 4) or ref.unit <= 0:
            fail(f'{label}: invalid field width/unit')
        if ref.field < 0 or ref.field+ref.width > len(source.data):
            fail(f'{label}: source field exceeds object size')
        start = offsets[ref.source] + ref.field
        occupied = set(range(start, start+ref.width))
        if fields & occupied:
            fail(f'{label}: overlapping relocation fields')
        fields |= occupied
        if ref.kind == 'size':
            value = len(target.data) + ref.addend
        else:
            if not 0 <= ref.addend <= len(target.data):
                fail(f'{label}: target addend exceeds object size')
            value = offsets[ref.target] + ref.addend
            if ref.kind == 'relative':
                if ref.base is not None:
                    resolve(ref.base)
                    value -= offsets[ref.base]
            elif ref.kind == 'segmented':
                if not 0 <= ref.segment <= 15 or value >= 0x1000000:
                    fail(f'{label}: segmented address overflow')
                value |= ref.segment << 24
            elif ref.kind in ('cpu', 'physical'):
                if not 0x80000000 <= address < 0x80400000:
                    fail(f'{label}: CPU base must lie in original 4 MB RAM')
                value += address
                if value >= 0x80400000:
                    fail(f'{label}: target exceeds original 4 MB RAM')
                if ref.kind == 'physical':
                    value -= 0x80000000
            elif ref.kind == 'rom':
                if stage != 'rom':
                    fail(f'{label}: ROM fixup before compression is an unsupported cross-stage dependency')
                value += address
            else:
                fail(f'{label}: unsupported address kind {ref.kind!r}')
        if value % ref.unit:
            fail(f'{label}: value is not divisible by offset unit {ref.unit}')
        value //= ref.unit
        if not 0 <= value < 1 << (8*ref.width):
            fail(f'{label}: value 0x{value:X} overflows {ref.width}-byte field')
        result[start:start+ref.width] = value.to_bytes(ref.width, 'big')
    return bytes(result), offsets
