#!/usr/bin/env python3
"""Generate award-rendering probes using injected progress after replay exit.

These exercise the three podium roots and their loaded asset bank. They do not
establish that normal race results satisfy the unlock requirements.
"""
import argparse
import copy
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True, help='new temporary scenario directory')
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    original = json.loads((root/'config/shift-tests/post-race-replay-exit.json').read_text())
    args.out.mkdir(parents=True, exist_ok=False)
    for award in range(3):
        scenario = copy.deepcopy(original)
        scenario.update(
            description=f'Pass award {award+1}: inject completed progress after partial-history replay exit; '
                        'test podium bank loading and rendering, not race unlock requirements',
            frames=[13350, 13500, 13700, 13900], stop_frame=14000,
            observe=[dict(symbol='gGameSaveDataBuffer', label='progression_level', offset=0x4c,
                          width=8, frames=[13500, 13700], expected=award+1),
                     dict(symbol='gRaceCameraModeChangeDisabled', width=8, expected=0),
                     dict(symbol='gMenuFadeAlpha', width=16)])
        scenario['writes'] += [
            dict(frame=13230, symbol='gRaceSplitscreenMode', width=8, value=0),
            dict(frame=13230, symbol='gGameSaveDataBuffer', offset=0x4c, width=8, value=award)]
        for course in ([0, 1, 2, 3, 4, 9] if award == 0 else [5 if award == 1 else 6]):
            scenario['writes'].append(dict(frame=13230, symbol='gGameSaveDataBuffer',
                                           offset=0x34+course, width=8, value=1))
        (args.out/f'award-{award+1}.json').write_text(json.dumps(scenario, indent=2)+'\n')


if __name__ == '__main__':
    main()
