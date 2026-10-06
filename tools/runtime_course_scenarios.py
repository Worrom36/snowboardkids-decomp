#!/usr/bin/env python3
"""Generate temporary course-load scenarios from the controller menu scenario.

The debugger selects a course during board selection, before race assets load.
This bypasses unlock requirements; it tests course loading and early gameplay,
not the unlock UI, complete races, or saves. Checked-in game content is untouched.
"""
import argparse
import copy
import json
from pathlib import Path

COURSES = ['Big Snowman', 'Sunset Rock', 'Night Highway', 'Grass Valley', 'Dizzy Land',
           'Quicksand Valley', 'Silver Mountain', 'Animal Land', 'Ninja Land', 'Rookie Mountain']


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    root = Path(__file__).resolve().parents[1]
    original = json.loads((root/'config/shift-tests/menus-and-race.json').read_text())
    a.out.mkdir(parents=True, exist_ok=False)
    for index, name in enumerate(COURSES):
        scenario = copy.deepcopy(original)
        scenario.update(description=f'{name} load and early single-player gameplay; debugger bypasses course unlocks',
                        frames=[6000, 7500, 9000], stop_frame=9100,
                        writes=[dict(frame=4900, symbol='gRaceCourseIndex', width=16, value=index)],
                        observe=[dict(symbol='gRaceCourseIndex', width=16, expected=index)])
        if index == 7:
            # Animal Land has no normal-race object table. Enter it through the
            # actual Skill Game / Trick Game menu path, which initializes its mode.
            scenario['description'] = 'Animal Land via the normal Skill Game / Trick Game controller menu path'
            scenario.pop('writes')
            for event in scenario['events']:
                if event['frame'] in (4000, 4400, 4600):
                    event['buttons'] = 0xb0000000  # stick Y = -80
            scenario['events'] += [dict(frame=frame, player=0, buttons=128, duration=8)
                                   for frame in (6000, 6200, 6400)]
        (a.out/f'course-{index:02d}.json').write_text(json.dumps(scenario, indent=2)+'\n')
    print(f'Generated {len(COURSES)} course scenarios under {a.out}')


if __name__ == '__main__':
    main()
