from itertools import product
from subprocess import run
import re

matrix = {
    'core': ['auto', 'dynamic', 'full', 'normal'],
    'fpu': ['true', 'false'],
    'cputype': ['auto', '386', '386_prefetch', 'pentium'],
    'resolution': ['320x200', '640x480'],
    'exe cpu': ['386', '586'],
    'exe config': ['debug', 'release'],
}

re_total = re.compile('^Total time \\(s\\): (\\d+(.\\d+)?)$')
re_time = re.compile('^Frame time \\(ms\\) Min: (\\d+) Avg: (\\d+) Max: (\\d+)$')
re_fps = re.compile('^Frames per second Min: (\\d+(.\\d+)?) Avg: (\\d+(.\\d+)?) Max: (\\d+(.\\d+)?)$')

def where(x):
    core, fpu, cputype, res, execpu, execonfig = x
    if execpu=='586' and fpu=='false': return False
    if execpu=='586' and cputype[:3]=='386': return False
    return True

matrix = product(*matrix.values())
matrix = [*filter(where, matrix)]
total = len(matrix)
num = 0
results = []

for i in matrix:
    core, fpu, cputype, res, execpu, execonfig = i
    num += 1
    cmd = [
        'dosbox-x',
        '-c', 'd:',
        '-c', 'cd boom3',
        '-c', f"build.dos\\boomdos\\{execonfig}.{execpu}\\boomdos.exe data\level.boo -P demo.dmo -k -m{res} > bench.txt",
        '-c', 'exit',
        '-set', f"cpu core={core}",
        '-set', f"cpu fpu={fpu}",
        '-set', f"cpu cputype={cputype}",
        '-set', 'cpu cycles=max'
    ]
    print(f"[{num}/{total}] {i}")
    p = run(cmd)
    if p.returncode != 0:
        print(f"Process returned: {p.returncode}")
        break
    r = list(i)
    with open("bench.txt", "r") as f:
        m = re_total.match(f.readline())
        r.append(m.groups()[0])
        m = re_time.match(f.readline())
        r += list(m.groups())
        m = re_fps.match(f.readline())
        r += list(m.groups()[::2])
    results.append(r)
    print(' '.join(r[len(i):]))

    if num == 2: break

with open("bench.csv", "w") as f:
    for r in results:
        f.write(';'.join(r) + '\n')