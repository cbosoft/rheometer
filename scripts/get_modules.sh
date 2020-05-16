SOURCE="$(ls src/control/controllers/*.c) $(ls src/control/setters/*.c)"
MODULES=$(echo $SOURCE | sed 's,src/control/,,g' | sed 's/\.c/\.so/g')
echo $MODULES
