#!/usr/bin/env python

"""
Tool to convert traditional include guards to #pragma once.

This tool is licensed under GPLv2.
See the LICENSE file for details.

Dowloaded from this: https://github.com/marcusmueller/include-guard-convert
Usage: In Linux/Mac Terminal (not Windows), go to the Source folder then execute the following command:

find . -iname "*.h" -exec python3 ~/Programming/include-guard-convert.py {} \;

Note: After the conversion for the header files, uncrustify need to be run to re-format them

"""

import re
import os
import subprocess
import shlex 

regexes = {
		'ifndef' : '^\s*#(?:ifndef|IFNDEF)\s+([A-Za-z_0-9]{4,})\s*$',
		'define' : '^\s*#(?:define|DEFINE)\s+([A-Za-z_0-9]{4,})\s*$',
		'endif' : '^\s*#(?:endif|ENDIF)\s*(/\*.*\*/|//.+)?\s*$',
		'blank' : '^\s*(/\*.*\*/|//.+)?\s*$',
		'pragma' : '^\s*#(?:pragma|PRAGMA)\s+(?:once|ONCE)'
}
patterns = dict( [ (key, re.compile(regexes[key]) ) for key in regexes.keys() ] )

cpp_commands = {
		'strip_comments' : 'cpp -w -dD  -fpreprocessed  -P -x c++ {file}',
		'test_if_guarded' : 'cpp -w -P -x c++ -D{define} {file}'
}

class guarded_include(object):
	"""
	Class representing an #include'able file (a.k.a. a header).
	"""

	def __init__(self, filename, autoconvert = False):
		self.filename = filename
		assert(self._test_readable())
		if autoconvert and self.test_oldstyle_guarded():
			self.convert()

	def _test_readable(self):
		return os.access(self.filename, os.R_OK)


	def test_oldstyle_guarded(self):
		try:
			self._stripped = subprocess.check_output(
					shlex.split(
							cpp_commands['strip_comments'].format(file=self.filename)
							),
					stderr=subprocess.STDOUT
					).decode('utf-8')
			lineend = self._stripped.find('\n')
			match_ifndef = patterns['ifndef'].search(self._stripped[:lineend])
			if not match_ifndef:
					return False
			define = match_ifndef.group(1)
			with_define = subprocess.check_output(
					shlex.split(
							cpp_commands['test_if_guarded'].format(file=self.filename, define=define)
							),
					stderr=subprocess.STDOUT
					)
			if not len(with_define) < 2:
				return False
			fh = open(self.filename, 'r')
			line = fh.readline()
			while not patterns['ifndef'].search(line):
				line = fh.readline()
				if not len(line):
					fh.close()
					return False

			line = fh.readline()
			fh.close()
			return patterns['define'].search(line)

		except subprocess.CalledProcessError as err:
			return False

	def convert(self):
		freadh = open(self.filename, 'r')
		lines = freadh.readlines()
		sep = '\r\n' if lines[0].endswith('\r\n') else '\n'
		freadh.close()
		fwriteh = open(self.filename , 'w')
		for l_number in range(1,len(lines)):
			line = lines[-l_number]
			if patterns['blank'].search(line):
				continue
			elif patterns['endif'].search(line):
				lines.pop(-l_number)
				break
			else:
				raise SyntaxError('encountered meaningful line after last #endif: \n'+line)
		define = None
		done = False
		for l_number,line in enumerate(lines):
			if done:
				fwriteh.write(line)
				continue
			if define is None:
				pattern = patterns['ifndef']
			else:
				pattern = patterns['define']
			match = pattern.search(line)
			if match:
				newdefine = match.group(1)
				if define is None:
					define = newdefine
				elif define == newdefine:
					fwriteh.write('#pragma once' + sep)
					done = True
			else:
				fwriteh.write(line)
		if define is None:
			raise SyntaxError('could not find #ifndef')
		elif define != newdefine:
			raise SyntaxError('found #ifndef ' + define + ', does not match #define ' + newdefine)
		fwriteh.close()

if __name__ == '__main__':
	import argparse
	parser = argparse.ArgumentParser()
	parser.add_argument('filename', nargs='*')
	args = parser.parse_args()
	for filen in args.filename:
		try:
			gi = guarded_include(filen)
			if gi.test_oldstyle_guarded():
				gi.convert()
		except SyntaxError as e:
			print (filen)
			print (e)
