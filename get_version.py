import subprocess as sp
from datetime import datetime

def runsh(command):
    pr = sp.Popen(command, shell=True, stdout=sp.PIPE, stderr=sp.PIPE)
    pr.wait()
    so = pr.stdout.read().decode().split('\n')[:-1]
    se = pr.stderr.read().decode().split('\n')[:-1]
    return so, se

branch, __ = runsh("git branch | grep \* | cut -d ' ' -f2")
branch = branch[0]
date_of_last_commit, __  = runsh("git --no-pager log -1 --format=%cd")
date_of_last_commit = date_of_last_commit[0]

dt = datetime.strptime(date_of_last_commit, '%a %b %d %H:%M:%S %Y %z')
version_name = dt.strftime('%Y%m%d')

if branch == 'dev':
    version_name = f'{version_name}[dev]'
else:
    version_name = f'{version_name}'

print(version_name)
