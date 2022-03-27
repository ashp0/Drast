#
# Created by Ashwin Paudel on 2022-03-23.
#

import glob
import subprocess
import sys
import time


class Colors:
    header = '\033[95m'
    ok_cyan = '\033[96m'
    ok_green = '\033[92m'
    warning = '\033[93m'
    failed = '\033[91m'
    bold = '\033[1m'
    reset = '\033[0m'


EXECUTABLE = '../../build/drast'
DRAST_FILES = glob.glob('*.drast')
FILES_AND_TIMES = []

START_TIME = time.time()

for file in DRAST_FILES:
    try:
        # Run the program and also get the time token
        start = time.time()
        program_output = subprocess.check_output([EXECUTABLE, file])
        end = time.time()

        # Append the file and time to the list
        FILES_AND_TIMES.append((file, end - start))

        # If the program is specified to print output, print it
        if 'verbose' in sys.argv:
            print(program_output.decode())

    except subprocess.CalledProcessError as e:
        # If a file fails, print the error
        print(f"{e.output}")
        print(f"{Colors.failed}Failed to run {file}{Colors.reset}")
        exit(1)

# Print the time taken to run all tests
print(f"{Colors.header}Total Time Taken: {time.time() - START_TIME} seconds{Colors.reset}")

# Print the results and each file and time taken
for files_and_times in sorted(FILES_AND_TIMES):
    print(
        f"{Colors.ok_green}Test Passed: '{Colors.reset}{Colors.bold}{files_and_times[0]}{Colors.ok_green}' :: '{Colors.ok_cyan}{files_and_times[1]}{Colors.ok_green}' {Colors.reset}")
