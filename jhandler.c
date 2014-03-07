#include "juse.h"
#include "jframework.h"

jFrameContext *__juse_context = NULL;

int AcceptNewPeer (jFrameContext *juse, jPeer *peer)
{
    int client_fd;
    struct sockaddr_in caddr;
    int sklen = sizeof (caddr);
    jPeer *new_peer;

    client_fd = accept (peer->sock_fd, (struct sockaddr*)&caddr, &sklen);
    if (client_fd > 0) {
        new_peer = juse->peer_manager.new_peer ();
        if (new_peer) {
            new_peer->sock_fd = client_fd;
            new_peer->inet_info.proto = INETProtocolTCP;
            new_peer->inet_info.addr  = caddr.sin_addr.s_addr;
            new_peer->inet_info.port  = ntohs (caddr.sin_port);

            DC_list_add (&juse->peers, (void*)new_peer);
            return 0;
        } else {
            close (client_fd);
            return -1;
        }
    }
    return -1;
}

void RecvHandler (jFrameContext *juse)
{
    
}


void SendHandler (jFrameContext *juse)
{
}


void CoreHandler (jFrameContext *juse)
{
    int ret;
    int max_fd;
    fd_set fds;
    jPeer *peer;
    BOOL reflag = TRUE;

#ifdef OS_WINDOWS
#else
    
#endif
    while (juse->sig_event != JuseEventExitSystem) {
        if (reflag) {
            FD_ZERO (&fds);
            for (i=0; i<juse->peers.count; i++) {
                peer = DC_list_object_at_index (&juse->peers, i);
                FD_SET (peer->sock_fd, &fds);
            }
            reflag = FALSE;
        }

        ret = select (max_fd+1, &fds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                break;
            }
        } else if (ret == 0) {
            ; //Do nothing
        } else if (ret > 0) {
            for (i=0; i<juse->peers.count && ret; i++) {
                peer = DC_list_object_at_index (&juse->peers, i);
                if (FD_ISSET (peer->sock_fd, &fds)) {
                    if ((peer == juse->local_peers[0] 
                        || peer == juse->local_peers[1])
                        && peer->inet_info.proto == INETProtocolTCP) {
                        reflag = (AcceptNewPeer (juse, peer)?FALSE:TRUE);
                    } else {
                    
                    }
                    ret--;
                }
            }
        }
    }
}


void JuseEventHandler (jFrameContext *juse, int sig)
{
    switch (sig) {
        case JuseEventRecv:
            RecvHandler (juse);
            break;
        case JuseEventSend:
            SendHandler (juse);
            break;
        case JuseEventRun:
            CoreHandler (juse);
            break;
        case JuseEventRebuildPeers:
        default:
            break;
    }
}
