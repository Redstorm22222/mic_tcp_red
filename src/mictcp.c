#include <mictcp.h>
#include <api/mictcp_core.h>
#define SIZE 5

int setId = 0;
 mic_tcp_sock globalSockets[SIZE]; 
/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(0);
   mic_tcp_sock newSock;
   newSock.fd = setId;
   newSock.state = IDLE;
   globalSockets[setId] = newSock;
   setId++;

   return newSock.fd;
}

/*
 * Permet d’attribuer une adresse à un socket.
 * Retourne 0 si succès, et -1 en cas d’échec
 */
int mic_tcp_bind(int socket, mic_tcp_sock_addr addr)
{
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   mic_tcp_sock sockRecup = globalSockets[socket];
   sockRecup.local_addr = addr;
   return 0;
   // -1 si out ouf bounds mais géré dans mic_tcp_socket
}

/*
 * Met le socket en état d'acceptation de connexions
 * Retourne 0 si succès, -1 si erreur
 */
int mic_tcp_accept(int socket, mic_tcp_sock_addr* addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    /* mic_tcp_sock sockRecup = globalSockets[socket];
    sockRecup.state = SYN_RECEIVED;
    mic_tcp_pdu pdu_syn_ack;
    pdu_syn_ack.header.source_port = sockRecup.remote_addr.port;
    pdu_syn_ack.header.dest_port = (*addr).port;
    pdu_syn_ack.header.syn = 1;
    pdu_syn_ack.header.ack = 1;
    IP_send(pdu_syn_ack, addr->ip_addr); */
    return 0;
}

/*
 * Permet de réclamer l’établissement d’une connexion
 * Retourne 0 si la connexion est établie, et -1 en cas d’échec
 */
int mic_tcp_connect(int socket, mic_tcp_sock_addr addr)
{
    printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
    globalSockets[socket].remote_addr = addr;
    return 0;

    /*pdu_syn.header.dest_port = addr.port;
    pdu_syn.header.syn = 1;

    IP_send(pdu_syn, addr.ip_addr);

    mic_tcp_pdu pdu_recu;

    IP_recv(&pdu_recu, &addr.ip_addr, &addr.ip_addr, 1000);

    if (pdu_recu.header.syn == 1 && pdu_recu.header.ack == 1){

        mic_tcp_pdu pdu_ack;

        pdu_ack.header.source_port = sockRecup.remote_addr.port;
        pdu_ack.header.dest_port = addr.port;
        pdu_ack.header.ack = 1;

        IP_send(pdu_ack, addr.ip_addr);

        sockRecup.state = ESTABLISHED;

        return 0;

    } else {

        return -1;
    } */

    
}

//remote = connect
//local = bind

/*
 * Permet de réclamer l’envoi d’une donnée applicative
 * Retourne la taille des données envoyées, et -1 en cas d'erreur
 */
int mic_tcp_send (int mic_sock, char* mesg, int mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");

    // récupérer le socket à partir de son identifiant, pour avoir la structure mic_tcp_sock associée
    mic_tcp_sock sockRecup = globalSockets[mic_sock];
    mic_tcp_pdu pdu;
    pdu.payload.data = mesg;
    pdu.payload.size = mesg_size;
    //Pas besoin du nul port local ou localhost
    pdu.header.dest_port = sockRecup.remote_addr.port;
    int sent_data = IP_send(pdu, sockRecup.remote_addr.ip_addr);
    return sent_data;
}

/*
 * Permet à l’application réceptrice de réclamer la récupération d’une donnée
 * stockée dans les buffers de réception du socket
 * Retourne le nombre d’octets lu ou bien -1 en cas d’erreur
 * NB : cette fonction fait appel à la fonction app_buffer_get()
 */
int mic_tcp_recv (int socket, char* mesg, int max_mesg_size)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
    mic_tcp_payload payload;
    payload.data = mesg;
    payload.size = max_mesg_size;
    int effectivement_written = app_buffer_get(payload);
    return effectivement_written;
}

/*
 * Permet de réclamer la destruction d’un socket.
 * Engendre la fermeture de la connexion suivant le modèle de TCP.
 * Retourne 0 si tout se passe bien et -1 en cas d'erreur
 */
int mic_tcp_close (int socket)
{
    printf("[MIC-TCP] Appel de la fonction :  "); printf(__FUNCTION__); printf("\n");
    mic_tcp_sock sockRecup = globalSockets[socket];
    sockRecup.state = CLOSED;
    
}

/*
 * Traitement d’un PDU MIC-TCP reçu (mise à jour des numéros de séquence
 * et d'acquittement, etc.) puis insère les données utiles du PDU dans
 * le buffer de réception du socket. Cette fonction utilise la fonction
 * app_buffer_put().
 */
void process_received_PDU(mic_tcp_pdu pdu, mic_tcp_ip_addr local_addr, mic_tcp_ip_addr remote_addr)
{
    printf("[MIC-TCP] Appel de la fonction: "); printf(__FUNCTION__); printf("\n");
            app_buffer_put(pdu.payload);

}
