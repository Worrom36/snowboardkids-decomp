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
    p.add_argument('--extended-cpu-race', action='store_true',
                   help='generate one complete Big Snowman CPU-driven race with a results assertion')
    a = p.parse_args()
    root = Path(__file__).resolve().parents[1]
    original = json.loads((root/'config/shift-tests/menus-and-race.json').read_text())
    a.out.mkdir(parents=True, exist_ok=False)
    if a.extended_cpu_race:
        original.update(
            description='Extended Big Snowman race with player-zero CPU control after setup; '
                        'debugger bypasses unlocks. Full course path and results, not human steering coverage.',
            frames=[7500, 10000, 14000, 18000, 22000, 26000], stop_frame=26100,
            writes=[dict(frame=4900, symbol='gRaceCourseIndex', width=16, value=0),
                    dict(frame=6000, symbol='gRacePlayers', offset=4, width=8, value=1)],
            observe=[dict(symbol='gRaceCourseIndex', width=16, expected=0),
                     dict(symbol='gRacePlayers', label='player_flags', offset=0x2fc, width=32),
                     dict(symbol='gRacePlayers', label='lap', offset=0x508, width=8),
                     dict(symbol='gRacePlayers', label='course_path', offset=0x502, width=16),
                     dict(symbol='gRaceResultState', width=8, frames=[26000], expected=1),
                     dict(symbol='gMenuFlowState', width=32)])
        (a.out/'full-cpu-race.json').write_text(json.dumps(original, indent=2)+'\n')
        print(f'Generated extended CPU race under {a.out}')
        return
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
