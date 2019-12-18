# dca-t10
Repositorio para la práctica del tema 10 de DCA.

He agregado un bug que a veces salta a veces no, depende de las versiones de los compiladores.
El bug consiste en calcular el modulo '((l+m)%M) para poder simular el comportamiento de un toroide en un array, teoricamente debería de funcionar, pero se me ha presentado el caso de que en algún ordenador me ha saltado una violación de segmento debido a que la operación de modulo devolvia negativos.

Como usar bisect:
	https://apiumhub.com/es/tech-blog-barcelona/git-bisect/
Pintar la gráfica de commits y comprobar en cual estoy:
	git log --graph --pretty=format:'%Cred%h%Creset -%C(yellow)%d%Creset %s %Cgreen(%cr) %C(bold blue)<%an>%Creset' --abbrev-commit

