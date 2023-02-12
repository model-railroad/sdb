## SDB Module Base Class.

class SdbModule:

    def __init__(self, manager, name):
        # - manager: reference to ModuleManager singleton.
        # - name: a 2-letter module name.
        self._manager = manager
        self._name = name
        self._manager.register(self)
        self._msg_queue = []

    def name(self):
        # Invoked by ModuleManager to get the module 2-letter name.
        return self._name

    def enqueue(self, msg):
        # Invoked by ModuleManager to add data to the msg queue.
        self._msg_queue.append(msg)

    def onStart(self):
        # Returns: nothing.
        pass

    def onLoop(self):
        # Returns: float seconds before next loop.
        if self._msg_queue:
            pass # handle pending messages
        # Return how many seconds max to wait till next loop.
        return 1.0

##
