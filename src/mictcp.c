#include <mictcp.h>
#include <api/mictcp_core.h>
#define SIZE 5

int setId = 0;
char PE = 0;
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
    printf("1\n");
    pdu_syn_ack.header.dest_port = (*addr).port;
    printf("2\n");

    pdu_syn_ack.header.syn = 1;
    printf("3\n");

    pdu_syn_ack.header.ack = 1;
    printf("4\n");

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
    /* mic_tcp_sock sockRecup = globalSockets[socket];

    mic_tcp_pdu pdu_syn;
    pdu_syn.header.dest_port = addr.port;
    pdu_syn.header.syn = 1;

    while (sockRecup.state != ESTABLISHED){

        IP_send(pdu_syn, addr.ip_addr);

        mic_tcp_pdu pdu_recu;
        mic_tcp_ip_addr addr_recu_local;
        mic_tcp_ip_addr addr_recu_remote;

        int res = IP_recv(&pdu_recu, &addr_recu_local, &addr_recu_remote, 1000);

        if (res != -1){
            if (pdu_recu.header.syn == 1 && pdu_recu.header.ack == 1){

                mic_tcp_pdu pdu_ack;

                pdu_ack.header.source_port = sockRecup.remote_addr.port;
                pdu_ack.header.dest_port = addr.port;
                pdu_ack.header.ack = 1;

                IP_send(pdu_ack, addr.ip_addr);

                sockRecup.state = ESTABLISHED;

                return 0;
            //reprise en cas de reception d'un nouveau synack
            } 
        } 
    }*/

    globalSockets[socket].remote_addr = addr;
    return 0;
    
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
    pdu.header.seq_num = PE;

    PE = (PE + 1) % 2;

    sockRecup.state = WAIT_ACK;
    int timeout = 1000;

    int sent_data;

    //TODO : Dans le test on ne sort pas de cette boucle, peut-être pb avec PE

    while (sockRecup.state == WAIT_ACK){

        sent_data = IP_send(pdu, sockRecup.remote_addr.ip_addr);
        printf("oui %d \n", sent_data);
        mic_tcp_pdu pdu_ACK;
        pdu_ACK.payload.size = 0;
        mic_tcp_ip_addr addr_recu_local;
        mic_tcp_ip_addr addr_recu_remote;
        printf("avant ip rcv \n");
        int resultat = IP_recv(&pdu_ACK,&addr_recu_local,&addr_recu_remote,timeout);
        printf("après ip rcv \n");

        if (resultat == -1){
            timeout = 1000;
        } else if (pdu_ACK.header.ack == 1 && pdu_ACK.header.seq_num == PE){

            sockRecup.state = ESTABLISHED;

        }
    }

    
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
    printf("dans ip rcv avant app_buffer_get\n");
    int effectivement_written = app_buffer_get(payload);
    printf("dans ip rcv après app_buffer_get\n");
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

    mic_tcp_pdu pdu_FIN;

    pdu_FIN.header.dest_port = sockRecup.remote_addr.port;
    pdu_FIN.header.fin = 1;
    IP_send(pdu_FIN, sockRecup.remote_addr.ip_addr);

    mic_tcp_pdu pdu_recu;

    int timeout = 1000;

    mic_tcp_ip_addr addr_recu_local;
    mic_tcp_ip_addr addr_recu_remote;

    if (IP_recv(&pdu_recu, &addr_recu_local, &addr_recu_remote, timeout)!= -1 && pdu_recu.header.ack == 1){

        sockRecup.state = CLOSED;
        return 0;

    }else {

        return -1;
    }

    
    
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
            
    if (pdu.payload.size > 0 && pdu.header.seq_num == PE){

        app_buffer_put(pdu.payload);
        pdu.header.seq_num = (pdu.header.seq_num + 1) % 2;
        mic_tcp_pdu pdu_ACK;
        pdu_ACK.header.source_port = pdu.header.dest_port;
        pdu_ACK.header.dest_port = pdu.header.source_port;
        pdu_ACK.header.ack = 1;
        pdu_ACK.header.seq_num = pdu.header.seq_num;

        IP_send(pdu_ACK, remote_addr);
    }
}
