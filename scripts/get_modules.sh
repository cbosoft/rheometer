SOURCE="$(ls src/control/controllers/*.c) $(ls src/control/setters/*.c)"
MODULES=$(echo $SOURCE | sed 's,src/control/,,' | sed 's/\.c/\.so/')
echo $MODULES
