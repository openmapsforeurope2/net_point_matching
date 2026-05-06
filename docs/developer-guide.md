# Introduction

La présente documentation, à destination des développeurs, a pour objectif de présenter le détail du fonctionnement du processus de maintien de la cohérence entre une classe de type ponctuel et la classe de type réseau à laquelle elle est associée après que cette dernière ait été mise en cohérence aux frontières.

# Installation

## Code source 

Le code source de l'application est disponible sur le dépôt [area_matching](https://github.com/openmapsforeurope2/area_matching.git)

## Dépendances 

L'installation de l'application nécessite la compilation préalable de bibliothèques internes et externes à l'IGN.

Voici le graphe des dépendances :

<img src="images/dependencies.png" width="500" height="auto">

### Socle IGN 

Le socle logiciel de l'IGN regroupe un ensemble de bibliothèques développées en interne qui permettent d'unifier l'accès aux bibliothèques c++ de traitement et de stockage de données géographiques.
On y trouve notamment des modèles de données pivots (géométries, objet attributaire), des fonctions de lecture/écriture de conteneurs d'objets, des opérations sur les géométries, de nombreux algorithmes et outils spécifiquement conçus pour répondre à des problématiques géomaticiennes...

Le code source du socle ce trouve sur le dépôt [sd-socle](http://gitlab.forge-idi.ign.fr/socle/sd-socle.git)

### LibEPG 

Cette bibliothèque, développée à l'IGN et s'appuyant essentiellement sur le socle logiciel, contient de nombreux algorithmes et fonctions utilitaires dédiés spécifiquement aux besoins des produits européens (EGM/ERM) ainsi qu'au projet [OME2](https://github.com/openmapsforeurope2/OME2).
Elle comporte essentiellement des fonctions de généralisations, des fonctions utiles au management du processus tels que des utilitaires de log, d'orchestration, de gestion du contexte).
On y trouve également des opérateurs permettant d'encapsuler des objets géométriques complexes afin d'en optimiser la manipulation (par l'utilisation de graphes, d'indexes...) et ainsi d'accroitre les performances globales des processus.

Le code source de la bibliothèque libepg ce trouve sur le dépôt [libepg](https://github.com/IGNF/libepg.git)


# Fonctionnement du processus

Le traitement de mise en cohérence des objets ponctuels avec les données de type réseau associées est lancé pour un couple de pays frontaliers.

## Etapes préliminaires

Les données sur lesquelles ce traitement est lancé doivent avoir été nettoyées en amont à l'aide de l'outil **clean** du projet [data-tools](https://github.com/openmapsforeurope2/data-tools) qui permet de supprimer les objets trop éloignés de leur pays.
Cet outil doit être utilisé sur des tables de travail dans lesquelles sont extraites les données des deux pays à traiter autour de leur frontière commune.


## Principe général du traitement

Le processus de mise en cohérence des données de type ponctuel avec les données de type réseau est décomposé en une succession d'étapes clés.
Afin d'orchestrer l'enchainement de ces étapes l'application utilise l'outil **epg::step::StepSuite** de la bibliothèque **libepg**. Ce dernier permet de lancer une succession de **epg::step::Step** dans lesquels sont implémentés les traitements de chaque étape.
Un code (numéro à trois chiffres) est attribué à chaque étape. Les étapes sont ordonnancées selon cette numérotation. Si une étape transforme les données sur lesquelles elle travaille, une ou plusieurs tables dédiées préfixées du code de l'étape sont créées. Ces créations sont réalisées en copiant les tables d'une étape antérieure (qui n'est pas nécessairement l'étape immédiatement antérieure, car toutes les étapes ne travaillent pas sur les mêmes données).
Ce fonctionnement permet de conserver les résultats intermédiaires du processus. Cela donne la possibilité d'arrêter et de reprendre le traitement en cours de processus et facilite le travail de d'analyse et de deboggage.


Les étapes qui composent le traitement de raccordement sont les suivantes :

**510** - création d'une table d'adjacence établissant les relations entre les données ponctuelles avec les données réseaux avant mise en cohérence
<br>
**520** - mise en cohérence des données ponctuelles avec les données réseaux raccordées aux frontières.


L'outil **epg::step::StepSuite** donne la possibilité de ne lancer que certaines étapes ou une plage de plusieurs étapes.

## Configuration

L'outil s'appuie sur de nombreux paramètres de configuration permettant d'adapter le comportement des algorithmes en fonctions des spécificités nationales (sémantique, précision, échelle, conventions de modélisation...).

On trouve dans le [dossier de configuration](https://github.com/openmapsforeurope2/area_matching/tree/main/config) les fichiers suivants :

- epg_parameters.ini : regroupe des paramètres de base issus de la bibliothèque libepg qui constitue le socle de développement l'outil. Ce fichier est aussi le fichier chapeau qui pointe vers les autres fichiers de configurations.
- db_conf.ini : informations de connexion à la base de données.
- theme_parameters_hydro_node.ini : configuration des paramètres spécifiques au traitement des données de la classe __hydro_node__.
- theme_parameters_road_node.ini : configuration des paramètres spécifiques au traitement des données de la classe __road_node__.


## Lancement du traitement

L'outil s'utilise en ligne de commande.
Le traitement peut être lancé sur deux types de données surfaciques :
- noeud hydrographique (code hydro_node)
- noeud routier (code road_node)

<br>

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


## Les étapes - fonctionnement détaillé

Préalablement au lancement des étapes du processus est créée, si cette dernière n'existe pas déjà, une table d'adjacence, composée de deux colonnes : [identifiant de point] et [identifiant d'arc]. Le nom de cette table est la concaténation du nom de la table d'objets ponctuels traités _POINT_TABLE_ avec le suffix _ADJACENCY_TABLE_SUFFIX_.

### 510 : InitNetPointAdjacency

L'objectif de cette étape est de créer une table d'adjacence permettant d'établir les liens, le cas échéant, entre les objet ponctuels et les arcs du réseau associé. Un point est associé à un arc, s'il est suffisamment proche d'une de ses extrémités.

#### Données de travail :

| table                          | entrée | sortie | entitée de travail | description                                                    |
|--------------------------------|--------|--------|--------------------|----------------------------------------------------------------|
| ADJACENCY_TABLE                |        | X      |                    | table d'adjacence                                              |
| NET_TABLE_INIT                 | X      |        |                    | table du réseau associé avant mise en cohérence aux frontières |

#### Principaux opérateurs de calcul utilisés :
- app::calcul::AdjacencyTableInitializationOp

#### Description du traitement :
Paramètre utilisés: 
| paramètre                       | description                                                                                      |
|---------------------------------|--------------------------------------------------------------------------------------------------|
| AI_MAX_ASSOCIATION_DIST         | distance maximum entre un objet ponctuel et les extrémités d'un arc pour établir une association |

Le traitement décrit ci-après et répété pour chacun des deux pays frontaliers.
On parcourt l'ensemble des arcs (avant raccordement aux frontières) du pays. Pour chaque arc, on récupère la géométrie de ses extremités sous forme d'un multi-point ainsi que son identifiant. Ce 'tuple' est enregistré et indexé spatialement.
On parcourt ensuite les objets ponctuels appartenant au pays traité. Pour chacun d'eux on récupère, grâce à l'index spatial construit précedemment, les tuples des arcs à proximité. Pour chaque tuple dont le multipoint est situé à une distance inférieur à _AI_MAX_ASSOCIATION_DIST_ de l'objet ponctuel, on enregistre une association {identifiant de l'objet pontuel, identifiant de l'arc}.

Une fois collectées l'ensemble des associations possibles pour les deux pays on les enregistre de manière persistante dans la table d'adjacence.


### 520 : PointMatching

La première étape ayant permis de décrire topologiquement (par relation d'incidence) les noeuds du réseau auquel sont liés les objets ponctuels, cette seconde étape a pour objectif d'identifier ce même noeud dans le réseau modifié par le processus de raccordement (qui peut engendrer des déplacements ou suppression d'arcs).

#### Données de travail :

| table                          | entrée | sortie | entitée de travail | description                                                    |
|--------------------------------|--------|--------|--------------------|-------------------------------------------------------------------------------|
| POINT_TABLE                    | X      | X      | X                  | table des objet ponctuels traitée                                             |
| NET_TABLE_MATCHED              | X      |        |                    | table du réseau associé résultant du processus de raccordement aux frontières |
| ADJACENCY_TABLE                | X      |        |                    | table d'adjacence                                                             |

#### Principaux opérateurs de calcul utilisés :
- app::calcul::PointMatchingOp

#### Description du traitement :
Paramètre utilisés: 
| paramètre                       | description                                                                                                                 |
|---------------------------------|-----------------------------------------------------------------------------------------------------------------------------|
| PM_MAX_MATCHING_DIST            | distance maximum d'un noeud du réseau raccordé à un objet ponctuel pour qu'il soit selectionné comme candidat à l'appairage |


Dans un premier temps on charge en mémoire la table d'adjacence, à chaque objet ponctuel, pour lequel un ou plusieurs liens ont pu être établis avec les arcs du réseau, on associe la liste de ses arcs adjacents (les arcs ne possédant aucun lien avec le réseau n'ont pas été enregistrés dans la table d'adjacence).

On répète ensuite le traitement qui suit pour chacun des deux pays frontaliers.
On charge un graphe géométrique simplifié avec le réseau du pays, chaque arc étant représenté par segment un défini par ses deux extrémités.
Ensuite on parcourt l'ensemble des objets ponctuels. Pour chaque objet présent dans la liste d'adjacence on récupère les noeuds du graphe situés à une distance inférieure à _PM_MAX_MATCHING_DIST_ ainsi que la liste des arcs adjacents _L1_ (avant raccordement). Tous ces noeuds sont candidats à l'association avec l'objet ponctuel. Il nous faut maintenant selectionner celui qui représente l'association la plus pertinente. Pour cela on parcourt les noeuds candidats et pour chacun on récupère la liste des arcs incidents _L2_ (après raccordement). Si le nombre d'items de _L2_ est supérieur à celui _L1_, on élimine le candidat (le raccordement au frontières ne peut pas avoir créé de nouvelles connexions au sein d'un même pays), sinon on calcule le nombre d'items communs entre _L1_ et _L2_ et on calcul le score affecté au candidat comme suit :
_score_ = _D_ * ( _N1_ / _N_ )     
  avec : 
  - _D_ : distance entre l'objet ponctuel et le candidat
  - _N1_ : nombre d'items de _L1_
  - _N_ : nombre d'items communs entre _L1_ et _L2_

Si aucun candidat n'est éligible à l'association, l'objet ponctuel est supprimé (l'objet possédait initialement une connexion au réseau qui a disparu à l'issu du processus de raccordement). Sinon on déplace l'objet ponctuel vers meilleur candidat si la distance qui les sépare n'est pas nulle.
