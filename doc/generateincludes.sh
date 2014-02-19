ls -lisa codeexamples/*.rst &&
ls codeexamples/*.rst | sed "s/^/.. include:: /" > codeexamplesincludes.rst && 
echo "Succesfully generated codeexamplesincludes.rst" && 
echo "now you can use \"make <format>\" to generate output!"