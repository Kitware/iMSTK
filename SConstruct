#SConstruct
#This file contains content adapted from the miniAT project's SConstruct file
#Project home: https://bitbucket.org/miniat/0x1-miniat

import os;

scripts = list()

#Generate a build environment
env = Environment()

#get debug argument
debug = ARGUMENTS.get('debug', 0)

#set the build type
if int(debug):
	buildtype = "debug"
else:
	buildtype = "release"

#define directories
root_dir = os.getcwd()
build_dir = os.path.join(root_dir, "build")
build_dir = os.path.join(build_dir, env['PLATFORM'])
build_dir = os.path.join(build_dir, buildtype)
include_dir = os.path.join(root_dir, "include")
src_dir = os.path.join(root_dir, "src")
install_dir = os.path.join(root_dir, "bin")
install_dir = os.path.join(install_dir, env['PLATFORM'])
install_dir = os.path.join(install_dir, buildtype)
dependency_dir = os.path.join(root_dir, "dependencies")
dependency_dir = os.path.join(dependency_dir, env['PLATFORM'])
dependency_dir = os.path.join(dependency_dir, buildtype)

#Setup platform specific stuff
#setup compiler flags
if env['PLATFORM'] == 'posix':
	#This assumes we are building with g++
	if int(debug):
		env.Append(CPPFLAGS = '-g ')

	#add color to gcc
	env.Append(CPPFLAGS = '-fdiagnostics-color=always ')
	env.Append(CPPFLAGS = '-std=c++11 ')
	env.Append(CPPFLAGS = '-fpermissive ')
elif env['PLATFORM'] == 'win32':
	if int(debug):
		env.Append(CPPFLAGS = ['/Zi', '/INCREMENTAL:NO', '/DEBUG:Yes', '/MDd', '/Od'])
		env.Append(LINKFLAGS = ['/OPT:REF', '/OPT:ICF'])
	else:
		env.Append(CPPFLAGS = ['/MD', '/O2'])

	env.Append(CPPDEFINES = ['GLUT_BUILDING_LIB', 'WIN32'])
	env.Append(CPPFLAGS = ['-EHsc'])

#Setup the build directory and source directory.
#duplicate = 0: Don't duplicate the source files in to the build directory
env.VariantDir(build_dir, src_dir, duplicate = 0)

#define exports for other SCons files
export = [ 'env', 'build_dir', 'include_dir', 'src_dir', 'install_dir', 'dependency_dir', 'export', 'buildtype' ]
#add the include directory to the global includes
if 'CPPPATH' in env:
	env['CPPPATH'].append(include_dir)
else:
	env.Append(CPPPATH = [ include_dir ])

#The following is adapted from the miniAT project:
#https://bitbucket.org/miniat/0x1-miniat/src/7386cb22e1ec2ba5c304c5ae91a7d6957583fd85/SConstruct?at=master
# As per the terms of the MIT license:
# Copyright (C) 2008--2014 Miniat Project
# Copyright (C) 2009--2013 Francesco Nidito
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Find and execute all the SConscript files
for root, dirnames, filenames in os.walk(root_dir):
	for filename in filenames:
		# If it is a SConscript file, append it to the list of
		# scripts found
		if "build" not in root and filename == "SConscript":
			t = os.path.join(root, filename)
			m = root.replace(root_dir, "")
			m = os.path.join(build_dir, m.lstrip(os.sep))
			scripts.append((t, m))
			print("Including: " + t)

for (script, module) in scripts:
	q = os.path.join(root_dir, build_dir, module)
	print("Executing script: " + q)
	SConscript(script, variant_dir = q, exports = export)