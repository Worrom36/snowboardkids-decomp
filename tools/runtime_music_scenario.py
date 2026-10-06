#!/usr/bin/env python3
"""Generate a temporary scenario requesting music through the game's own queue."""
import argparse
import json
from pathlib import Path


def scenario(banks):
    root = Path(__file__).resolve().parents[1]
    menu = json.loads((root/'config/shift-tests/menus-and-race.json').read_text())
    result = dict(
        description='Stationary menu: request music through the pending-music queue; '
                    '600 VI frames per bank, assert playback after 30 frames because short jingles finish early',
        controllers=1, frames=[], observation_frames=[], stop_frame=3000+600*len(banks)+10,
        events=[event for event in menu['events'] if event['frame'] <= 2800], writes=[], observe=[])
    for index, bank in enumerate(banks):
        start = 3000+600*index
        result['frames'].append(start+500)
        result['observation_frames'].append(start+30)
        result['writes'].extend([
            dict(frame=start, symbol='gPendingMusicCommandArg', width=32, value=bank),
            dict(frame=start, symbol='gPendingMusicCommand', width=32, value=1)])
        for symbol, value in [('gCurrentMusicSequenceBank', bank), ('gPendingMusicCommand', 0),
                              ('gMusicSequenceStopped', 0)]:
            result['observe'].append(dict(symbol=symbol, width=32, frames=[start+30], expected=value,
                                         label=f'bank_{bank}_{symbol}'))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out', type=Path, required=True, help='new temporary JSON file')
    parser.add_argument('--bank', type=int, action='append', choices=range(19),
                        help='specific runtime bank; repeat for a transition test (default: all 17 distinct banks)')
    args = parser.parse_args()
    banks = args.bank if args.bank is not None else [*range(8), *range(10, 19)]
    if len(banks) != len(set(banks)):
        parser.error('duplicate bank observations require distinct scenario identities')
    args.out.parent.mkdir(parents=True, exist_ok=True)
    with args.out.open('x') as output:
        json.dump(scenario(banks), output, indent=2)
        output.write('\n')


if __name__ == '__main__':
    main()
