# AISE_Project
## Made by DIAS Nicolas and LAPLANCHE Alexis

## Dependancies:

### Libunwind:
```bash
	apt install libunwind-dev
```

### awk:
```bash
	apt install awk
```

### grep:
```bash
	apt install grep
```

## Utilisation

### Compilation
```bash
	make compile
```

### Lancement
```bash
	./dbg ./"program a débugguer"
```

### Fonctionnement

Une fois le programme lancé des aides sont affiché afin de donner les fonctionnalitées présentes.

Pour commencé il faut dire au debugger le programme que vous souhaitez observer, pour cela vous pouvez le donner lors du lancement du debugger ainsi que ses arguments (ex: "./main ./crash 1 2") ou alors utiliser attach une fois le debugger lancé et donné le programme ainsi que ses arguments (ex: "attach ./crash 1 2").

Une fois cette étape terminé il est possible de lancer le programme avec r ou run.

Une fois le programme terminé il est possible d'utiliser "bt" ou "backtrace" ce qui va lancer un backtrace, ce qui les fonctions se trouvant dans la pile dans son etat actuel. La commande "reg" ou "register" ce qui affichera la valeur des registres. La commande "info" qui permet d'affiché des informations basique du programme tel que le pid, le pid du fils ou le gid ainsi que l'emplacement du programme. La commande "p" ou "prev" permet de revenir en arrière dans l'exécution su programme. La commande "reset" permet de réinitialiser le curseur de la pile. La commande "clear" permet de nettoyer le terminal et de réafficher la liste des commandes. Et enfin la commance "quit" permet de quitter le debugger.

## Explication

Notre projet est un debugger il permet de lancer un programme et de suivre son comportement au long de son execution.
Pour cela le programme va regarder le signal renvoyé par le programme lors qu'il se termine et va ensuite regarder dans la pile afin de récupérer les informations liée au programme.
Pour cela nous avons utilisé la bibliothèque "libunwind". Libuwind "wrap" l'appel système ptrace et fourni une api plus simple d'utilisation. On se sert de libunwind pour:
- Réaliser un backtrace sur notre programme
- Récupérer le contenu des registres

Pour la section info, nous avons parsé le retour de l'outils "llvm-dwarfdump" a l'aide de grep et de awk. Nous avons récupérer le retour de llvm-dwarfdump de la manière suivante:
- Récupéré la ligne où commence la section concernant le .c de notre programme (c'est a dire pas les parties qui concernent les librairies externe)
- Parser pour récupérer les sections qui nous intéresse (variable, subprogram(fonction), paramètre des fonctions) a partir de la ligne calculé précedemment
- Nous avons ensuite parser les sections pour les mettres dans des listes chainées pour facilité le stockage/la lecture 

