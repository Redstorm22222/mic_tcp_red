## README BE-RESEAU / Favard Eloi - CAJAL Alexis

Pour compiler mictcp et générer les exécutables des applications de test depuis le code source fourni, taper :

	make

Deux applications sont fournies, tsock_texte et tsock_video, elles peuvent être lancées soit en mode puits, soit en mode source selon la syntaxe suivante :

	Usage: ./tsock_texte [-p|-s destination] port
	Usage: ./tsock_video [-p|-s] [-t (tcp|mictcp)]

Les versions 1,2 et 3 sont implémentées et sont fonctionnelles.

Choix d’implémentation :

-> Stop & Wait
  - À chaque envoi de PDU, l’émetteur attend un accusé de réception (ACK) avant d’envoyer le suivant.  
  - Simple à mettre en œuvre, permet de garder un contrôle fin sur la séquence et l’ordre des paquets.
-> Fiabilité partielle
  - Contrairement à une fiabilité “tous pertes récupérées” (comme dans TCP), on autorise une perte contrôlée de paquets tant que le taux de pertes effectif reste inférieur à un seuil fixé (`loss_permitted = 5%`).

Mécanisme de négociation des pertes (MICTCP‑v3) :

-> Fenêtre glissante (taille fixe 10)

  - On maintient un tableau window[WINSIZE] où chaque entrée vaut 1 si le paquet correspondant a été accusé, 0 sinon.
  - A chaque nouvel envoie on calcule taux_pertes = (nb paquets perdus / WINSIZE)
  - Si taux_pertes < loss_permitted (5 %), on autorise de laisser passer le paquet suivant sans forcer un ré-essai immédiat. Sinon, on rentre en mode Stop & Wait classique jusqu’à réception de l’ACK.

-> Bénéfices de MIC-TCP‑v3 par rapport à TCP standard

  - En lâchant temporairement les pertes faibles (jusqu’à 5 %), on évite des rétransmissions coûteuses qui dégradent le débit sur réseaux instables.
  - Le protocole s’ajuste en temps réel au taux de pertes observé, sans intervention manuelle de la couche application.
  - Moins de ré-essais pour de petites pertes, donc une réduction des délais d’envoi & réception.