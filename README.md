# Objets communicants – Projet UWB (Stella + Portenta)
Automne 2025

## Pour exécuter le projet :

- ajouter dans l'IDE Arduino les librairies **StellaUWB**, **ArduinoBLE** et **PortentaUWBShield** ;
- exécuter le script `post_install.sh` si cela n'a pas déjà été fait pour d'autres projets ;
- choisir une adresse pour chaque appareil avant d’y téléverser le code (exemple : `0x11:0x11` et `0x12:0x12` pour les balises, `0x07:0x07` pour le contrôleur).
- lancer uwb-joystick.py qui va lire sur le port série les mesures reportées par le Portenta et les interpréter en commandes de joystick
- lancer pingpong.py qui va lancer le jeu de pong.

## Plan du projet

- Analyse des besoins
- Conception (plan en V)
- Implémentation (code)
- Tests unitaires
- Validation

**Rendu :**

- Fichiers source + cahier labo (Git)
- Vidéo démo du projet
- Présentation

---

# Rapport de projet — Capture de mouvement UWB

## Contexte et objectif

- Développer une preuve de concept de capture / gestuelle courte portée en utilisant un **Portenta H7 + UWB Shield** comme contrôleur et **2 modules Stella UWB** comme balises.
- Capacité visée : suivre le déplacement d’un objet (Portenta en main) entre deux balises.
- Contraintes : faible latence (réactivité gestuelle), simplicité d’intégration (peu de calibration).
- Les mesures de distance sont peu précises (entiers en cm avec un jitter important), mais suffisantes pour détecter des mouvements grossiers.

À l’origine, nous voulions détecter des gestes par **occultation du signal UWB** (*passive sensing*), sans appareil dans la main et idéalement en **2D ou plus**. Cependant, la librairie du Portenta UWB Shield est encore incomplète : certaines fonctions avancées, comme l’accès au **RSSI**, ne sont pas exposées de manière stable. De plus, le support du **multicast** entre le contrôleur et plusieurs balises n’a été ajouté sur le dépôt Git de la librairie que deux semaines avant la date de rendu. Nous avons donc dû **réduire et adapter nos objectifs** en conséquence, pour nous concentrer sur un suivi de position **actif en 1D**.

## Matériel et réseau UWB

- **Contrôleur** : Portenta H7 avec UWB Shield (rôle *Controller / Initiator* en multicast).
- **Balises** : 2 × Stella UWB (rôle *Tag / Responder / Controlee*), adresses courtes exemplaires `0x11:0x11` et `0x12:0x12`, contrôleur `0x07:0x07`.
- **Librairies** : `StellaUWB` et `PortentaUWBShield` qui implémentent la mesure de distance (TWR – Two Way Ranging) et la session contrôleur / balises (multicast).

L'installation est la suivante : deux balises font office de référentiel spatial, fixées sur un support stable (table). Le Portenta est tenu en main et déplacé latéralement entre les deux balises.

## Architecture logicielle

- Les balises rejoignent une session multicast en tant que **responders** et publient les mesures de distance via Serial  
  (`sketch-stella/sketch-stella.ino`).
- Le contrôleur initie les échanges TWR et agrège les distances pour estimer la position relative (1D).  
  Référence : `sketch-portenta-shield/sketch_nov27a/sketch_nov27a.ino`.
- Handler de ranging : filtrage sur `status == 0` et distance valide (`!= 0xFFFF`), impression des distances en centimètres sur le port série (115200 bauds).

---

# Idée de scénario gestuel

- Session UWB entre les deux Stella et le contrôleur.
- Identification des deux Stella (gauche / droite).
- Échantillonnage périodique des distances.
- Buffer circulaire de plusieurs mesures pour lisser le signal.
- À chaque nouveau point, comparaison avec les précédents pour détecter un mouvement.
- Gestes envisagés : déplacement latéral (gauche / droite), en avant / arrière (proximité), arrêt.
- Publication de la position estimée (gauche / milieu / droite) par l'interface série USB.
- Un programme externe (Python / Processing) lit la position et lui associe une action.
- Cette action est utilisée dans un petit jeu ou une interaction visuelle simple.

---

## Protocoles d’essai

### 1. Actif (suivi de position 1D)

- Disposer deux Stella de part et d’autre (gauche / droite).
- Tenir le Portenta et le déplacer sur l’axe latéral.
- Observer en série la variation des deux distances pour classer la position (gauche / milieu / droite) et détecter une trajectoire (geste).

### Résumé pratique des tests

1. **Test actif** : déplacer le Portenta de gauche à droite devant les Stella → observer les distances et la trajectoire estimée (geste).

---

## Implémentation actuelle

- Code balise (Stella) : session multicast, callback de ranging, logs des distances.
- Code contrôleur (Portenta) : initialisation UWB Shield, démarrage de la session multicast, réception des mesures, filtrage et affichage.
- Mesures disponibles : distance (cm) à chaque échange TWR.

## Résultats et observations

- Les distances lues en série sont **cohérentes pour des trajectoires lentes** lorsque `status == 0` (TWR valide).
- Le **jitter** est significatif (quelques centimètres) mais reste suffisant pour distinguer clairement trois zones : proche de la balise gauche, au milieu, proche de la balise droite.
- Le scénario **actif 1D** permet donc de :
  - détecter un déplacement gauche → droite ou droite → gauche ;
  - estimer une position qualitative (gauche / milieu / droite) en temps réel.

## Limites et risques

- Géométrie 2D / 3D non résolue : la configuration actuelle est principalement 1D (entre deux balises).
- Sensibilité au placement et à l’orientation des antennes ; multi-trajets possibles en environnement indoor.
- Jitter important à courte distance, nécessitant un filtrage et une hystérésis pour éviter les faux changements d’état.
- Gestion des adresses et des sessions à renforcer (éviter les collisions en cas de plusieurs sessions UWB voisines).

## Prochaines étapes

- Ajouter, si la librairie le permet dans une future version, l’affichage de `rx_power` dans le handler côté contrôleur et calibrer automatiquement un seuil (moyenne glissante « à vide »).
- Implémenter une classification simple (gauche / milieu / droite) avec hystérésis pour lisser le bruit, puis publier la position sur Serial et éventuellement sur BLE.
- Logger un jeu de données court (distance + timestamp, et RSSI si disponible) pour documenter précision et robustesse.
- Intégrer la position dans un petit jeu ou une visualisation interactive (Python / Processing) pour la vidéo démo et la présentation, en mettant en avant le scénario actif de suivi 1D.