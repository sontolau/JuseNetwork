#include "jframework.h"

void DataProc (jFrameContext *fc, fd_t *fds)
{
    
}

void IOProc (jFrameContext *fc, int num, fd_t *fds)
{
    int i;
    int mod_cli;
    jMetaModule *mod;
    int cmd;
    int mod_id;
    int mod_reply[2] = {0, 0};
    char strmod[10] = {0};
    unsigned int datalen = 0;
    jNetBuf *nb;

    if (FD_ISSET (fc->io_sock, fds)) {
        num--;
        mod_cli = accept (fc->io_sock, NULL, NULL, 0);
        if (mod_cli > 0 (mod = (jMetaModule*)calloc (1, sizeof (jMetaModule)))) {
            mod->io_sock = mod_cli;
            FD_SET (mod_cli, fds);
            DC_list_add (&fc->mod_list, (void*)mod);
        }
    }

    
    for (i=0; num && i<fc->mod_list.count; i++) {
        mod = DC_object_at_index (i);
        if (FD_ISSET (mod->io_sock, fds)) {
            num--;
            if (recv (mod->io_sock, &cmd, sizeof (int), 0) >= sizeof (int)) {
                mod_reply[0] = cmd;

                switch (cmd) {
                    case MCTL_REG:
                        if (recv (mod->io_sock, &mod_id, sizeof (int), 0) < sizeof (mod_id)) {
err_close:
                            close (mod->io_sock);
                            FD_CLR (mod->io_sock, fds);
                            if (mod->module_id > 0) {
                                sprintf (strmod, "%lu", mod_id);
                                DC_list_remove_object (&fc->mod_list, (void*)mod);
                                DC_dict_remove_object_with_key (&fc->proc_modules, (char*)strmod);
                            }
                            free (mod);
                        } else {
                            sprintf (strmod, "%lu", mod_id);
                            mod->module_id = mod_id;
                            DC_dict_add_object_with_key (&fc->proc_modules, (char*)strmod, mod);
                        }
                        
                        break;
                    case MCTL_UNREG:
                        if (mod->module_id > 0) {
                            sprintf (strmod, "%lu", mod_id);
                            DC_dict_remove_object_with_key (&fc->proc_module, (char*)strmod);
                        }
                        break;
                    case MCTL_SEND:
                        if (recv (mod->io_sock, &datalen, sizeof (int), 0) < sizeof (int)) {

                        } else {
                            if (datalen > fc->config.general.max_trans_size) {
                                mod_reply[1] = {MERR_OVER_BUFSIZE};
                            } else {
                                nb = fc->netbuf_manager.alloc_buffer (&fc->netbuff_manager, datalen);
                                if (nb == NULL) {
                                    mod_reply[1] = {MERR_NO_MORE_BUF};
                                } else {
                                    nb->length = recv (mod->io_sock, datalen, nb->data, 0);
                                    if ((int)nb->length <= 0) {
                                        fc->netbuf_manager.free_buffer (&fc->netbuf_manager, nb);
                                        goto err_close;
                                    } else {
                                        
                                    }
                                }
                            }
                        }
                        break;
                    default:
                        mod_reply[1] = {MERR_INVALID_CMD};
                        break;
                }

                if (send (mod->io_sock, mod_reply, sizeof (mod_reply), 0) < sizeof (mod_reply)) {
                    goto err_close;
                }
            } else {
                goto err_close;
            }
        }
    }
}

int ServerCore (int *fd_list, int num, void (*core)(jFrameContext*, int, fd_t*), jFrameContext *data)
{
    fd_t fds;
    int ret;
    struct timeval tm_wait = {0, 0};
    int i, maxfd=0;

    FD_ZERO (&fds);
    for (i=0; i<num; i++) {
        FD_SET (fd_list[i], &fds);
        maxfd = (maxfd > fd_list[i]?maxfd:fd_list[i]);
    }

    while (1) {
        tm_wait.tv_sec = 1;
        ret = select (maxfd + 1, &fds, NULL, NULL, &tm_wait);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            }
        } else if (ret == 0) {

        } else {
            core (data, ret, &fds);
        }
    }
}

void DataServerCore (jFrameContext *fc)
{
    int  fds[] = {fc->data.sock};

    ServerCore (fds, 1, DataProc, fc);
}



void RMIServerCore (jFrameContext *fc)
{
    int fds[] = {fc->rmi_sock};

    ServerCore (fds, 1, RMIProc, fc);
}


void IOServerCore (jFrameContext *fc)
{
    int fds[] = {fc->io_sock};
    DC_list_init (&fc->mod_list, NULL);
    DC_dict_init (&fc->proc_modules, NULL);
    ServerCore (fds, 1, IOProc, fc);
    DC_dict_destroy (&fc->proc_modules);
    DC_list_destroy (&fc->mod_list);
}
