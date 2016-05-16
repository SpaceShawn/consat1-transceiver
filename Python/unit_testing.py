import unittest

from transclib import * 

class TestTransclibMethods(unittest.TestCase):

    def test_SC_computeFletcher(self):
        self.assertEqual('foo'.upper(),'FOO')

suite = unittest.TestLoader().loadTestsFromTestCase(TestTransclibMethods)
unittest.TextTestRunner(verbosity=2).run(suite)
