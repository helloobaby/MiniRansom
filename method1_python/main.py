import os

os.environ.get('User')

for root, ds, fs in os.walk('USERNAME'):
        for f in fs:
            print(os.path.join(root,f))