default: gksweb.class
	java gksweb

applet: gksweb.class
	appletviewer -J-Djava.security.policy=java.policy gksweb.html

gksweb.class: gksweb.java
	javac $?

clean:
	rm -f *.class
