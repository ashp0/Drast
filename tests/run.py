#
# Created by Ashwin Paudel on 2022-03-23.
#

import sys
import glob
import time
import subprocess


class Colors:
    header = '\033[95m'
    ok_cyan = '\033[96m'
    ok_green = '\033[92m'
    warning = '\033[93m'
    failed = '\033[91m'
    bold = '\033[1m'
    reset = '\033[0m'


# Headers
EXECUTABLE = '../build/drast'
DRAST_FILES = glob.glob('*.drast')
FILES_AND_TIMES = []

# Start Time ( To calculate total time taken to run all tests )
program_start_time = time.time()

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
            print(program_output.decode('utf-8'))

    except subprocess.CalledProcessError as e:
        # If a file fails, print the error
        print(e.output.decode('utf-8'))
        print(Colors.bold + Colors.failed +
              'Test failed: `' + file + '`' + Colors.reset)
        exit(1)

# Print the time taken to run all tests
print(Colors.bold + Colors.ok_green + 'Finished running all tests in ' +
      str(time.time() - program_start_time) + ' seconds.' + Colors.reset)

# Print the results and each file and time taken
for files_and_times in sorted(FILES_AND_TIMES):
    print(Colors.ok_cyan + '\t`' + files_and_times[0] + '`, Time:',
          files_and_times[1].__format__('.2f'), 'seconds' + Colors.reset)
