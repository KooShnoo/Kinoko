import json
import os
import urllib.request

with open("lc_300_ghosts.json") as f:
    gor = json.load(f)


for fiwl in gor:
    urllib.request.urlretrieve(fiwl, "rkgs/" + os.path.basename(fiwl))