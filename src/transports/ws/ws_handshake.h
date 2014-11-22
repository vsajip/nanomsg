/*
    Copyright (c) 2013 250bpm s.r.o.  All rights reserved.
    Copyright (c) 2014 Wirebird Labs LLC.  All rights reserved.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom
    the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#ifndef NN_WS_HANDSHAKE_INCLUDED
#define NN_WS_HANDSHAKE_INCLUDED

#include "../../transport.h"

#include "../../aio/fsm.h"
#include "../../aio/usock.h"
#include "../../aio/timer.h"

#include "../../utils/int.h"

/*  This state machine exchanges a handshake with a WebSocket client. */

/*  Return codes of this state machine. */
#define NN_WS_HANDSHAKE_OK 1
#define NN_WS_HANDSHAKE_ERROR 2
#define NN_WS_HANDSHAKE_STOPPED 3

/*  WebSocket endpoint modes that determine framing of Tx/Rx and
    Opening Handshake HTTP headers. */
#define NN_WS_CLIENT 1
#define NN_WS_SERVER 2

/*  A ws:// buffer for nanomsg is intentionally smaller than recommendation of
    RFC 7230 3.1.1 since it neither requires nor accepts arbitrarily large
    headers. */
#define NN_WS_HANDSHAKE_MAX_SIZE 4096

/*  Size of minimal valid handshare request and reply. This amount of bytes
    is read initially so that we don't have to read the whole handshake
    in one-byte-at-a-time manner. */
#define NN_WS_REQ_MIN_SIZE 150
#define NN_WS_REP_MIN_SIZE 16

/*  Expected Accept Key length based on RFC 6455 4.2.2.5.4. */
#define NN_WS_HANDSHAKE_ACCEPT_KEY_LEN 28

struct nn_ws_handshake {

    /*  The state machine. */
    struct nn_fsm fsm;
    int state;

    /*  Controls HTTP headers and behavior based on whether this peer is
        acting as a Client or a Server. */
    int mode;

    /*  Used to timeout opening handshake. */
    struct nn_timer timer;
    int timeout;

    /*  The underlying socket. */
    struct nn_usock *usock;

    /*  The original owner of the underlying socket. */
    struct nn_fsm_owner usock_owner;

    /*  Handle to the pipe. */
    struct nn_pipebase *pipebase;

    /*  Requested resource when acting as client. */
    const char* resource;

    /*  Remote Host in header request when acting as client. */
    const char* remote_host;

    /*  Opening handshake verbatim from client as per RFC 6455 1.3. */
    uint8_t opening_hs [NN_WS_HANDSHAKE_MAX_SIZE];

    /*  Monitor/control the opening recv poll. */
    int retries;
    int recv_pos;
    size_t recv_len;

    /*  Expected handshake fields from client as per RFC 6455 4.1,
        where these pointers reference the opening_hs. */
    const uint8_t *host;
    size_t host_len;

    const uint8_t *origin;
    size_t origin_len;

    const uint8_t *key;
    size_t key_len;

    const uint8_t *upgrade;
    size_t upgrade_len;

    const uint8_t *conn;
    size_t conn_len;

    const uint8_t *version;
    size_t version_len;

    /*  Expected handshake fields from client required by nanomsg. */
    const uint8_t *protocol;
    size_t protocol_len;

    /*  Expected handshake fields from server as per RFC 6455 4.2.2. */
    const uint8_t *server;
    size_t server_len;

    const uint8_t *accept_key;
    size_t accept_key_len;

    char expected_accept_key [NN_WS_HANDSHAKE_ACCEPT_KEY_LEN + 1];

    const uint8_t *status_code;
    size_t status_code_len;

    const uint8_t *reason_phrase;
    size_t reason_phrase_len;

    /*  Unused, optional handshake fields. */
    const uint8_t *uri;
    size_t uri_len;
    const uint8_t *extensions;
    size_t extensions_len;

    /*  Identifies the response to be sent to client's opening handshake. */
    int response_code;

    /*  Response to send back to client. */
    uint8_t response [512];

    /*  Event fired when the state machine ends. */
    struct nn_fsm_event done;
};

/*  Structure that maps scalability protocol to corresponding
    WebSocket header values. */
struct nn_ws_sp_map {

    /*  Scalability Protocol ID... */
    int sp;

    /*  ... and corresponding WebSocket header field value. */
    const char* ws_sp;
};

void nn_ws_handshake_init (struct nn_ws_handshake *self, int src,
    struct nn_fsm *owner);
void nn_ws_handshake_term (struct nn_ws_handshake *self);
int nn_ws_handshake_isidle (struct nn_ws_handshake *self);
void nn_ws_handshake_start (struct nn_ws_handshake *self,
    struct nn_usock *usock, struct nn_pipebase *pipebase,
    int mode, const char *resource, const char *host);
void nn_ws_handshake_stop (struct nn_ws_handshake *self);

#endif

