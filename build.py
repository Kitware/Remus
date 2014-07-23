#!/usr/bin/env python

'''
CTest improperly reports the SHA of a pull request, as the commit doesn't exist
yet.  This script runs a CTest/Travis build properly, given the current SHA from
Travis via TRAVIS_COMMIT.
'''

import os, sys
import xml.etree.ElementTree as ET

#Generating Update.xml
os.system('ctest -D ExperimentalUpdate')

#Forcing revision to TRAVIS_COMMIT
tree = ET.parse('./Testing/Update.xml')
root = tree.getRoot()
revision = root.find('Revision')
revision.text = os.enviorn['TRAVIS_COMMIT']
tree.write('./Testing/Update.xml')

#Proceeding with the rest of the experimental build
code = os.system('ctest -D Experimental -j6 --schedule-random --track Travis')

#Exiting with the return code from ctest
sys.exit(code)