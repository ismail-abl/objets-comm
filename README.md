# objets-comm
    ✅ Détecter le mouvement de la main (Gestes Spatiaux) : Savoir si la main fait un "Swipe" vers la gauche, si elle "Pousse" vers l'avant, ou si elle dessine un cercle. C'est tout à fait faisable !

2. L'architecture suggérée (Le "Air Mouse" géant)

Pour réussir cela avec 2 Stellas et 1 Portenta, je vous conseille l'approche suivante : La Triangulation 1D ou 2D simplifiée.

Imaginez que vos deux capteurs Stella sont des points fixes (Ancres) posés sur une table, espacés de 50cm. Le Portenta est dans votre main (ou inversement).

La configuration :

    Stella A (Gauche) : Adresse 0x11 (Fixe)

    Stella B (Droite) : Adresse 0x22 (Fixe)

    Portenta (Main) : Maître (Mobile) - Il interroge A et B.























C'est un excellent plan de test. Vous allez explorer les deux extrêmes de la technologie UWB : la **précision temporelle** (pour les gestes actifs) et la **sensibilité radio** (pour le passif).

Voici comment structurer vos deux tests avec le matériel que vous possédez (2 Stellas fixes + 1 Portenta mobile/fixe).

-----

### 1\. Test "Gestes Spatiaux" (Active Triangulation)

**Configuration :**

  * **Stella A (Gauche)** : ID `1` (Fixe sur table)
  * **Stella B (Droite)** : ID `2` (Fixe sur table, à 50cm de A)
  * **Portenta (Main)** : Contrôleur mobile. Il mesure la distance vers A et B.

**Principe :** On compare l'évolution simultanée des deux distances.

**Code logique pour le Portenta :**
Vous devez mesurer les deux distances à chaque boucle. Voici la logique à insérer dans votre `loop` ou fonction de traitement.

```cpp
// Variables globales
float distA = 0, lastDistA = 0;
float distB = 0, lastDistB = 0;
#define SEUIL_MOUVEMENT 15 // cm (pour filtrer le bruit)

void analyserGeste() {
  float deltaA = distA - lastDistA;
  float deltaB = distB - lastDistB;

  // On ne réagit que si le mouvement est significatif
  if(abs(deltaA) > SEUIL_MOUVEMENT || abs(deltaB) > SEUIL_MOUVEMENT) {
    
    // Cas 1 : Les deux augmentent -> PULL (Reculer)
    if(deltaA > 0 && deltaB > 0) {
      Serial.println("GESTURE: PULL (Recul)");
    }
    // Cas 2 : Les deux diminuent -> PUSH (Avancer)
    else if(deltaA < 0 && deltaB < 0) {
      Serial.println("GESTURE: PUSH (Avance)");
    }
    // Cas 3 : A diminue, B augmente -> SWIPE GAUCHE
    else if(deltaA < 0 && deltaB > 0) {
      Serial.println("GESTURE: SWIPE GAUCHE <--");
    }
    // Cas 4 : A augmente, B diminue -> SWIPE DROITE
    else if(deltaA > 0 && deltaB < 0) {
      Serial.println("GESTURE: SWIPE DROITE -->");
    }
    
    // Mémorisation
    lastDistA = distA;
    lastDistB = distB;
  }
}
```

-----

### 2\. Test "Passive Sensing" (RSSI Blockage)

**Configuration :**

  * **Stella A** : Fixe sur un bord de porte/table.
  * **Portenta** : Fixe sur l'autre bord (face à Stella A).
  * **Main** : Vide. Elle passe **entre** les deux appareils.

**Principe :** Le corps humain est composé d'eau, qui absorbe très bien les fréquences UWB (6-8 GHz).

1.  Quand le chemin est libre : Signal fort (ex: -78 dBm).
2.  Quand la main coupe le faisceau : Signal chute (ex: -85 dBm) ou perte de paquet.

**Code pour afficher le RSSI (Puissance du signal) :**
La donnée RSSI est souvent cachée dans les structures de données UWB. Dans les librairies basées sur Decawave/Qorvo, elle s'appelle souvent `rx_power`.

Voici comment modifier l'affichage "Debug" précédent pour voir cette valeur :

```cpp
// Dans votre rangingHandler
if(twr[j].status == 0) {
    // Récupération de la puissance (nom variable selon la lib exacte, souvent rxPower ou rssi)
    // Si twr[j].rx_power existe :
    float signalStrength = twr[j].rx_power; 
    
    Serial.print("Dist: ");
    Serial.print(twr[j].distance);
    Serial.print(" cm | RSSI: ");
    Serial.print(signalStrength);
    Serial.println(" dBm");

    // LOGIQUE DE DÉTECTION PASSIVE
    // Seuil à calibrer : regardez la valeur moyenne "à vide" et soustrayez 5 ou 6 dBm
    float seuilCoupure = -84.0; 

    if(signalStrength < seuilCoupure) {
        Serial.println("ALERTE : OBSTACLE DÉTECTÉ (Baisse de signal) !");
        digitalWrite(LED_PIN, HIGH); // Allumer LED
    } else {
        digitalWrite(LED_PIN, LOW);
    }
}
```

**Note technique importante sur le RSSI :**
Les valeurs sont négatives.

  * **-75 dBm** est un signal **plus fort** que **-90 dBm**.
  * Donc, si le signal passe de -75 à -90, il a "baissé" (mais la valeur absolue 90 est plus grande que 75, attention aux conditions `if`).
  * *Logique mathématique :* `if (currentRSSI < referenceRSSI - 5)` (car -90 \< -80).

### Résumé des étapes pour vous :

1.  **Charger le code "Debug"** que je vous ai donné plus tôt.
2.  **Ouvrir le moniteur série**.
3.  **Test Passif d'abord :** Placez les capteurs fixes. Passez votre main au milieu. Regardez si une valeur change (soit le "Status" qui passe en erreur, soit le "RSSI" qui chute).
4.  **Test Actif ensuite :** Prenez le Portenta en main, déplacez-le de gauche à droite devant les Stellas et regardez les distances changer.

Lequel voulez-vous implémenter en premier ?