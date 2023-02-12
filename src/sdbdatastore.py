## SDB Data Store.

class SdbDataStore:

    def __init__(self):
        self._data = {}

    def put(self, key, value):
        self._data[key] = value

    def get(self, key, default=None):
        return self._data.get(key, default)

##
