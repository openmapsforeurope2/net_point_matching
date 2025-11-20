# net_point_matching

## Context

Open Maps For Europe 2 est un projet qui a pour objectif de développer un nouveau processus de production dont la finalité est la construction d'un référentiel cartographique pan-européen à grande échelle (1:10 000).

L'élaboration de la chaîne de production a nécessité le développement d'un ensemble de composants logiciels qui constituent le projet [OME2](https://github.com/openmapsforeurope2/OME2).


## Description

L'objectif du présent outil est :
 - dans un premier temps, d'établir une association entre les objets d'une classe de ponctuels avec les noeuds d'un graph formé par une classe de linéaires.
 - dans un second temps, après raccordement de la classe de linéaire aux frontière, d'assurer le maintient de la cohérence avec les objets ponctuels par déplacement de ces derniers.

L'application est lancée sur un couple de pays frontaliers pour lesquels un traitement de raccordement au frontière a été réalisé sur le réseau de linéaires associé à la classe de ponctuel que l'on souhaite mettre en cohérence.


## Fonctionnement

Le programme ne manipule pas directement les données de production. Les données à traiter, localisées autour de la frontière, sont extraites dans une table de travail. A l'issu du traitement les données dérivées sont injectées dans la table source en remplacement des données initiales.

Le processus de mise en cohérence est décomposé en plusieurs étapes. Un numéro est attribué à chaque étape. Une table de travail préfixée de ce numéro est délivrée en sortie de chaque étape. Chaque étape prend en données d'entrées les tables de travail générées lors d'étapes antérieures.

Voici la liste de l'ensemble des étapes dont dispose l'outil :

**510** - initialisation de la table d'adjacence
<br>
**520** - mise en cohérence des ponctuels par déplacement de ces derniers vers les noeuds associés


## Configuration

L'outil s'appuie sur de nombreux paramètres de configuration permettant d'adapter le comportement des algorithmes en fonctions des spécificités nationales (sémantique, précision, échelle, conventions de modélisation...).

On trouve dans le [dossier de configuration](https://github.com/openmapsforeurope2/net_point_matching/tree/main/config) les fichiers suivants :

- epg_parameters.ini : regroupe des paramètres de base issus de la bibliothèque libepg qui constitue le socle de développement l'outil. Ce fichier est aussi le fichier chapeau qui pointe vers les autres fichiers de configurations.
- db_conf.ini : informations de connexion à la base de données.
- theme_parameters_road_node.ini : configuration des paramètres spécifiques à l'application pour la classe d'objet _road_node_.

## Utilisation

L'outil s'utilise en ligne de commande.

Paramètres:
* c [obligatoire] : chemin vers le fichier de configuration
* s [obligatoire] : suffix de la table de travail
* ns [obligatoire] : suffix des tables de travail du réseau
* t [obligatoire] : nom de la classe d'objet (hydro_node ou road_node)
* sp [obligatoire] : étape(s) à executer (exemples: 510 ; 510,520 ; 510-520)
* arguments libres [obligatoire] : codes des deux pays frontaliers

<br>

Exemple d'appel pour lancer successivement l'ensemble des étapes sur la frontière franco-belge :
~~~
bin/net_point_matching --c path/to/config/epg_parameters.ini --t hydro_node --s 20251113 --ns 20251111 fr be
~~~

Exemple d'appel pour ne lancer qu'une seule étape :
~~~
bin/net_point_matching --c path/to/config/epg_parameters.ini --t hydro_node --s 20251113 --ns 20251111 --sp 510 fr be
~~~
