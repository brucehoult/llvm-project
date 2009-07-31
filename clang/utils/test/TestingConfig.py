class TestingConfig:
    """"
    TestingConfig - Information on a how to run a group of tests.
    """
    
    @staticmethod
    def frompath(path):
        data = {}
        f = open(path)
        exec f in {},data

        return TestingConfig(suffixes = data.get('suffixes', []),
                             environment = data.get('environment', {}))

    def __init__(self, suffixes, environment):
        self.root = None
        self.suffixes = set(suffixes)
        self.environment = dict(environment)
        


