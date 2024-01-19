import os
import sys
import time
import subprocess

class WorkDir:
    def __init__(self, next):
        self.prev = os.getcwd()
        self.next = next

    def __enter__(self):
        os.chdir(self.next)

    def __exit__(self, *args, **kwargs):
        os.chdir(self.prev)

def test_file(filepath, cpython=False):
    if cpython:
        x, y = os.path.split(filepath)
        with WorkDir(x):
            return os.system("python3 " + y) == 0
    if sys.platform == 'win32':
        return os.system("main.exe " + filepath) == 0
    else:
        return os.system("./main " + filepath) == 0

def test_dir(path):
    print("Testing directory:", path)
    for filename in sorted(os.listdir(path)):
        if not filename.endswith('.py'):
            continue
        filepath = os.path.join(path, filename)
        print("> " + filepath, flush=True)

        if path == 'benchmarks/':
            _0 = time.time()
            if not test_file(filepath, cpython=True):
                print('cpython run failed')
                continue
            _1 = time.time()
            if not test_file(filepath): exit(1)
            _2 = time.time()
            print(f'  cpython:  {_1 - _0:.6f}s (100%)')
            print(f'  pocketpy: {_2 - _1:.6f}s ({(_2 - _1) / (_1 - _0) * 100:.2f}%)')
        else:
            if not test_file(filepath):
                print('-' * 50)
                print("TEST FAILED! Press any key to continue...")
                input()

print('CPython:', str(sys.version).replace('\n', ''))
print('System:', '64-bit' if sys.maxsize > 2**32 else '32-bit')

if len(sys.argv) == 2:
    assert 'benchmark' in sys.argv[1]
    test_dir('benchmarks/')
else:
    test_dir('tests/')

    # test interactive mode
    print("[REPL Test Enabled]")
    if sys.platform in ['linux', 'darwin']:
        cmd = './main'
    elif sys.platform == 'win32':
        cmd = '.\main.exe'
    else:
        cmd = None

    if cmd is not None:
        res = subprocess.run([cmd], encoding='utf-8', input=r'''
    def add(a, b):
    return a + b

    class A:
    def __init__(self, x):
        self.x = x

    def get(self):
        return self.x


    print(add(1, 2))
    print(A('abc').get())''', capture_output=True, check=True)
        res.check_returncode()
        assert res.stdout.endswith('>>> 3\n>>> abc\n>>> '), res.stdout

print("ALL TESTS PASSED")
