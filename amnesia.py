from collections import OrderedDict
import json
import sys


def replay(capacity, operations):
    cache = OrderedDict()
    results = []
    for op in operations:
        kind = op[0]
        key = op[1]
        if kind == "get":
            if key in cache:
                cache.move_to_end(key)
                results.append(cache[key])
            else:
                results.append(None)
        elif kind == "put":
            if capacity > 0:
                value = op[2]
                if key in cache:
                    cache[key] = value
                    cache.move_to_end(key)
                else:
                    if len(cache) >= capacity:
                        cache.popitem(last=False)
                    cache[key] = value
    return results


data = json.load(sys.stdin)
print(json.dumps(replay(data["capacity"], data["operations"]), separators=(",", ":")))
