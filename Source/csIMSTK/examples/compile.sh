#!/bin/bash
sed -i 's/\<modIMSTK\>/modIMSTK.so/g' modIMSTKPINVOKE.cs
sed -i 's/modIMSTK.*\.so/modIMSTK.so/g' modIMSTKPINVOKE.cs


mainFiles[1]="testGeometry.cs"
mainFiles[2]="pbdCloth.cs"
mainFiles[3]="pbdVolume.cs"
mainFiles[4]="pbdCollisionOneObject.cs"
mainFiles[5]="femDeformable.cs"
mainFiles[6]="rigidBody2.cs"

# main="testGeometry"
main="pbdCloth"
# main="pbdVolume"
# main="pbdCollisionOneObject"
# main="femDeformable"
# main="rigidBody2"

rm -f ${main}.exe

for f1 in *.cs; do
    keep=true
    for f2 in "${mainFiles[@]}"; do
        if [ "$f1" = "$f2" ]; then
            keep=false
        fi
    done
    if [ "$f1" = "${main}.cs" ]; then
        keep=true
    fi
    if $keep; then
        files="${files} ${f1}"
    fi
done

# for f in "${files[@]}"; do
#     echo "$f"
# done

csc ${files}
mono ${main}.exe
