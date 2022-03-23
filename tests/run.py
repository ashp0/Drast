#
# Created by Ashwin Paudel on 2022-03-23.
#

import glob
import time
import subprocess

drast_executable = '../build/drast'

test_files = glob.glob('*.drast')

files_and_times = []

start_time = time.time()
for test_file in test_files:
    try:
        start = time.time()
        subprocess.check_output([drast_executable, test_file])
        files_and_times.append((test_file, time.time() - start))
    except subprocess.CalledProcessError as e:
        print(e.output)
        print('\033[1m\033[91mTest failed: `' + test_file + '`\033[0m')
        exit(1)

print('\n\033[1m\033[92mFinished running all tests in ' +
      str(time.time() - start_time) + ' seconds.\033[0m')

for files_and_times in sorted(files_and_times):
    print('\t\033[96m`' + files_and_times[0] + '`, Time:',
          files_and_times[1].__format__('.2f'), 'seconds')
