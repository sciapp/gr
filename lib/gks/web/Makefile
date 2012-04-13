default: gksweb.class
	java gksweb

applet: gksweb.jar
	appletviewer -J-Djava.security.policy=java.policy gksweb.html

gksweb.class: gksweb.java
	javac $?

gksweb.jar: gksweb.class
	jar -cf gksweb.jar *.class gksfont.dat java.policy 
	rm -f *.class

clean:
	rm -f *.class
	rm -f *.jar
