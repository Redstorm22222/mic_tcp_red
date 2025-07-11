#include <mictcp.h>
#include <api/mictcp_core.h>
#define SIZE 5
#define WINSIZE 10
#define loss_rate 20.0
#define loss_permitted 5.0


int setId = 0;
char PE = 0;
mic_tcp_sock globalSockets[SIZE];
int windex = 0;
int window[WINSIZE];
/*
 * Permet de créer un socket entre l’application et MIC-TCP
 * Retourne le descripteur du socket ou bien -1 en cas d'erreur
 */
int mic_tcp_socket(start_mode sm)
{
   int result = -1;
   printf("[MIC-TCP] Appel de la fonction: ");  printf(__FUNCTION__); printf("\n");
   result = initialize_components(sm); /* Appel obligatoire */
   set_loss_rate(loss_rate);
   mic_tcp_sock newSock;
   newSock.fd = setId;
   newSock.state = IDLE;
   globalSockets[setId] = newSock;
   setId++;

   return newSock.fd;
}

void init_window(){
    // 1 is for a send, 0 for a loss 
    for (int i = 0; i<WINSIZE; i++){
        window[i] = 0;
    }

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
    init_window();
    globalSockets[socket].remote_addr = addr;
    return 0;
    
}

//remote = connect
//local = bind


char loss_allowed(){

    int sum = 0;
    for (int i = 0; i < WINSIZE; i++)
    {
        sum += window[i];
    }
    sum = WINSIZE - sum;
    float ratio = (float)sum / (float)WINSIZE;
    return (ratio < loss_permitted/100.0);

}


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
    pdu.header.ack = 0;
    pdu.header.syn = 0;
    pdu.header.dest_port = sockRecup.remote_addr.port;
    pdu.header.seq_num = PE;

    sockRecup.state = WAIT_ACK;
    int timeout = 10;

    //PDU ACK creation
    mic_tcp_pdu pdu_ACK;
    pdu_ACK.payload.size = 0;
    mic_tcp_ip_addr addr_recu_local;
    mic_tcp_ip_addr addr_recu_remote;

    int sent_data = IP_send(pdu, sockRecup.remote_addr.ip_addr);
    windex = (windex + 1) % WINSIZE;
    int recv = IP_recv(&pdu_ACK,&addr_recu_local,&addr_recu_remote,timeout);

    printf("RECV : %d, ACK : %d, SYN : %d, ACK_NUM : %d  |  RECV != -1, ACK = 1, SYN = 0, ACK_NUM = PE = %d \n",recv,pdu_ACK.header.ack, pdu_ACK.header.syn,pdu_ACK.header.seq_num,PE);

    if (recv != -1 && pdu_ACK.header.ack == 1 && pdu_ACK.header.syn == 0 && pdu_ACK.header.seq_num == PE){

        printf("BIEN RECU, TOUT VA BIEN \n");
        window[windex] = 1;

    } else {

        printf("PAS RECU, TOUT VA PAS BIEN \n");
        printf("islossallowaded %d \n",loss_allowed());
        window[windex] = 0;
        if (!loss_allowed()){

            while (1){

                recv = IP_recv(&pdu_ACK,&addr_recu_local,&addr_recu_remote,timeout);
                if (recv != -1 && pdu_ACK.header.ack == 1 && pdu_ACK.header.syn == 0 && pdu_ACK.header.seq_num == PE){

                    break;

                }
                sent_data = IP_send(pdu, sockRecup.remote_addr.ip_addr);

            }
            window[windex] = 1;

        }

    }
    PE = (PE + 1) % 2;
    return sent_data;

    /* //sockRecup.state == WAIT_ACK
    window[windex] = 1;

    while (IP_recv(&pdu_ACK,&addr_recu_local,&addr_recu_remote,timeout) != -1 && pdu_ACK.header.ack == 1 && pdu_ACK.header.ack_num == PE){
        
        if (loss_allowed()){
                printf("La perte a été autorisée\n");
                printf("renvoi du paquet\n");
                printf("PE = %d, seq_num = %d",PE,pdu_ACK.header.ack_num);
                window[windex] = 0;
                windex = (windex + 1) % WINSIZE;
                return sent_data;

            //windex = (windex + 1) % WINSIZE;
            //sockRecup.state = ESTABLISHED;
            
        }
        sent_data = IP_send(pdu, sockRecup.remote_addr.ip_addr);

    }  
    printf("Le paquet a été envoyé\n");
    printf("windex = %d\n",windex);
    window[windex] = 1;                
    windex = (windex + 1) % WINSIZE;
    PE = (PE + 1) % 2;
    

    
    return sent_data; */
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
    printf("PE = %d, seq_num = %d \n",PE,pdu.header.seq_num);

    if (pdu.payload.size > 0 && pdu.header.seq_num == PE){

        app_buffer_put(pdu.payload);
        
        mic_tcp_pdu pdu_ACK;
        pdu_ACK.header.source_port = pdu.header.dest_port;
        pdu_ACK.header.dest_port = pdu.header.source_port;
        pdu_ACK.header.ack = 1;
        pdu_ACK.header.syn = 0;
        pdu_ACK.header.seq_num = pdu.header.seq_num;

        IP_send(pdu_ACK, remote_addr);
        PE = (PE + 1) % 2;
    } else {

        mic_tcp_pdu pdu_ACK;
        pdu_ACK.header.source_port = pdu.header.dest_port;
        pdu_ACK.header.dest_port = pdu.header.source_port;
        pdu_ACK.header.ack = 1;
        pdu_ACK.header.syn = 0;
        pdu_ACK.header.seq_num = pdu.header.seq_num;
        IP_send(pdu_ACK, remote_addr);

    }
}
