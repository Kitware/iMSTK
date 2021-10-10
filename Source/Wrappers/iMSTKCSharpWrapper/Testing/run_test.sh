# How to run NUnit on Ubuntu: https://stackoverflow.com/questions/31038629/run-nunit-test-on-ubuntu-from-command-line.

export MONO_PATH=$(pwd)/NUnit.ConsoleRunner.3.12.0/tools:$(pwd)/NUnit.3.13.2/lib/net40
export LD_LIBRARY_PATH=/home/jianfeng/Documents/imstk/build_csharp/install/lib:$LD_LIBRARY_PATH

testName=$1
rm -f ${testName}.dll
iMSTKSharp=/home/jianfeng/Documents/imstk/build_csharp/install/include/iMSTKSharp
mcs ${testName}.cs ${iMSTKSharp}/*.cs -unsafe -debug -target:library -r:NUnit.3.13.2/lib/net40/nunit.framework.dll -out:${testName}.dll
mono ./NUnit.ConsoleRunner.3.12.0/tools/nunit3-console.exe ${testName}.dll -noresult

