#include <stdio.h>
#include <pbcommon.h>
#include <appcommon.h>
#include "appcmn1.h"
#include "session.h"

#define PB_MSG_DISP_ACCEPT 1
#define PB_MSG_DISP_INVALID 2
#define PB_MSG_DISP_READ_MORE 3
#define PB_MSG_DISP_EMPTY 4

typedef int (*PB_CB)(uv_stream_t *connection, PbMsg *pbmsg);

int validate_pb_msg_common(uint32_t *leftover_data_size,
        uint32_t *current_location, PbMsg *pbmsg);

void on_rpc_response_end(uv_write_t *req, int status) {
    if (status == -1) {
        fprintf(stderr, "error on_write_end");
    } else {
        // printf("Writing completed with status %d\n", status);
        printf("Writing completed. data: %p, Freeing memory %p\n",
                req->data, req);
    }
    free(req);
}

PB_CB pbcbs[MT_MAX];

PB_CB cbfuncs(uint32_t payload_type) {
    if (payload_type > 0 && payload_type <= MT_MAX)
        return pbcbs[payload_type];
    else
        return NULL;
}

PB_CB register_cb(uint32_t payload_type, PB_CB func) {
    if (payload_type > 0 && payload_type <= MT_MAX) {
        pbcbs[payload_type] = func;
    }
}

int process_msg(uv_stream_t *connection, ssize_t siz, uv_buf_t buf) {
    printf("Processing %ld bytes incoming\n", siz);
    // the received data is in buf.base
    int leftover_data_size = siz;
    int current_location = 0;
    // process all messages in the received data stream
    while (leftover_data_size >= sizeof (PbHeader)) {
        PbMsg *pb_msg;
        pb_msg = (PbMsg *) &(buf.base)[current_location];
        uint32_t packet_length, packet_protocol;
        uint32_t payload_type;
        // LLG(LOG_DEBUG, "Header Length: %u", pb_msg->)
        packet_length = pb_msg->hdr.length;
        packet_protocol = pb_msg->hdr.protocol;
        payload_type = pb_msg->hdr.payload_type;

        // simple header validation
        if (packet_length < sizeof (PbHeader)) {
            printf("Error in the Protobuf message header length %u Protocol %u\n",
                    packet_length, packet_protocol);
            leftover_data_size = 0;
            break;
        }

        if (leftover_data_size < packet_length) {
            // incomplete protobuf data, we have to receive more to 
            // parse correctly
            break;
        }

        if (packet_length == sizeof (PbHeader)) {
            // empty message found, skip it
            printf("Empty message received message header length %u Protocol %u\n",
                    packet_length, packet_protocol);
            leftover_data_size -= packet_length;
            current_location += packet_length;
            continue;
        }

        // at ths point, leftover_data_size is >= pbmsg.hdr.length 
        // TODO use the paylaod_type to figure out how to parse the information.

        int payload_length = packet_length - sizeof (PbHeader);

        printf("Length of the header: %d protocol: %d payload_type: %u, siz %u\n",
                packet_length, packet_protocol, payload_type, payload_length);

        PB_CB cbfunc = cbfuncs(payload_type);
        if (cbfunc != NULL) {
            int rcb = cbfunc(connection, pb_msg);
        }


        leftover_data_size -= packet_length;
        current_location += packet_length;
    }

    // unprocessed data at the end
    // 100 bytes received, 56 bytes processed, and now we have 44 bytes left to process
    if (leftover_data_size > 0) {
        printf("We still have %u bytes to process\n", leftover_data_size);
    }

    return (leftover_data_size);
}



int handle_glc_reply(uv_stream_t *, PbMsg *) {
    LLG(LOG_DEBUG, "Received reply for logging configuration successfully..");
    return 0;
}

int handle_glc_req(uv_stream_t *connection, PbMsg *pbmsg) {
    int32_t packet_length, packet_protocol;
    uint32_t payload_type;
    packet_length = pbmsg->hdr.length;
    packet_protocol = pbmsg->hdr.protocol;
    payload_type = pbmsg->hdr.payload_type;
    uint32_t payload_length = packet_length - sizeof (PbHeader);

    bool rc;
    GlobalLogConfig global_log_config;
    rc = global_log_config.ParseFromArray(pbmsg->message, payload_length);
    if (rc) {
        LLG(LOG_INFO, "Logging global severity level %u", global_log_config.log_level());
        printf("Global log level: %d\n", global_log_config.log_level());
        printf("Byte size after parsing: %d\n", global_log_config.ByteSize());
    } else {
        // there was a parsing error
        // there was not enough data in the message
        printf("Error in parsing the protobuf message. Skip to the next message anyway\n");
    }
    
    set_global_log_severity(global_log_config.log_level());

    A1_GlobalLogConfig response_glc;
    response_glc.set_log_level(2);
    char *msg_buf = response_glc.make_msg();
    uint32_t msg_buf_size = response_glc.get_msg_size();
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);
    printf("Message total size: %u\n", msg_buf_size);

    //uv_stream_t *tcp = c->app_server_connection->handle;
    uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
    printf("Sending data using response %p\n", write_req);
    write_req->data = connection->data; // store the connection as callback data
    int buf_count = 1;
    uv_write(write_req, connection, &buf, buf_count, on_rpc_response_end);
    return 0;
}

int validate_pb_msg_common(PbMsg *pbmsg, uint32_t *leftover_data_size,
        uint32_t *current_location) {

    uint32_t packet_length, packet_protocol;
    uint32_t payload_type;
    packet_length = pbmsg->hdr.length;
    packet_protocol = pbmsg->hdr.protocol;
    payload_type = pbmsg->hdr.payload_type;

    // simple header validation
    if (packet_length < sizeof (PbHeader)) {
        printf("Error in the Protobuf message header length %u Protocol %u\n",
                packet_length, packet_protocol);
        *leftover_data_size = 0;
        return PB_MSG_DISP_INVALID;
    }

    if (*leftover_data_size < packet_length) {
        // incomplete protobuf data, we have to receive more to 
        // parse correctly
        return PB_MSG_DISP_READ_MORE;
    }

    if (packet_length == sizeof (PbHeader)) {
        // empty message found, skip it
        printf("Empty message received message header length %u Protocol %u\n",
                packet_length, packet_protocol);
        *leftover_data_size -= packet_length;
        *current_location += packet_length;
        return PB_MSG_DISP_EMPTY;
    }

    return 0;
}

int handle_publish_req(uv_stream_t *connection, PbMsg *pbmsg) {
    LLG(LOG_DEBUG, "Received a publish message");
    int32_t packet_length, packet_protocol;
    uint32_t payload_type;
    packet_length = pbmsg->hdr.length;
    packet_protocol = pbmsg->hdr.protocol;
    payload_type = pbmsg->hdr.payload_type;
    uint32_t payload_length = packet_length - sizeof (PbHeader);

    bool rc;
    A1_PublishedItems request;
    rc = request.ParseFromArray(pbmsg->message, payload_length);
    if (rc) {
        printf("Byte size after parsing: %d\n", request.ByteSize());
        printf("Objects published  %d\n", request.items_size());
        for (int i = 0; i < request.items_size(); i++) {
            PublishedItem *item = request.mutable_items(i);
            printf("Published MT: %d %d %d\n", item->item_id(), item->item_instance_start(),
                    item->item_instance_end());
        }
    } else {
        // there was a parsing error
        // there was not enough data in the message
        printf("Error in parsing the protobuf message. Skip to the next message anyway\n");
    }
    
    A1_PublishedItemsResult response;
    response.set_result(0);
    char *msg_buf = response.make_msg();
    uint32_t msg_buf_size = response.get_msg_size();
    
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);
    printf("Message total size: %u\n", msg_buf_size);

    //uv_stream_t *tcp = c->app_server_connection->handle;
    uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
    printf("Sending data using response %p\n", write_req);
    write_req->data = connection->data; // store the connection as callback data
    int buf_count = 1;
    uv_write(write_req, connection, &buf, buf_count, on_rpc_response_end);
    return 0;
}

int handle_publish_reply(uv_stream_t *connection, PbMsg *pbmsg) {
    
    int32_t sid, rid;
    sid = pbmsg->hdr.session_id;
    rid = pbmsg->hdr.random_id;
    printf("A publish reply was received sid: %u random: %u\n", sid, rid);
    LLG(LOG_DEBUG, " Standard reply received sid: %u random: %u\n", sid, rid);
    
}


int handle_std_reply(uv_stream_t *connection, PbMsg *pbmsg) {
    uint32_t sid, magic;
    sid = pbmsg->hdr.session_id;
    magic = pbmsg->hdr.random_id;
    printf("***---*** A std reply was received sid: %u random: %u\n", sid, magic);
    LLG(LOG_DEBUG, " Standard reply received sid: %u random: %u\n", sid, magic);
    
    ObSessionInfo_t ob_session;
    int rc = find_stored_session(sid, magic, &ob_session); 
    if (rc == 0) {
        // stored session has been found, execute the callback
        printf("Outbound stored session has been found, execute the callback.\n");
        rc = ob_session.cb(ob_session.data, connection, pbmsg);
        printf("Outbound session callback completed. Returning session ID\n");
        return_sid(sid);
    } else {
        printf("Stored session not found. Must have timed out.\n");
    }
}

int start_new_timer(uv_stream_t *connection, uint32_t sid, uint32_t rid);

int handle_delayed_response_req(uv_stream_t *connection, PbMsg *pbmsg) {
    LLG(LOG_DEBUG, "Received a request for delayed response");
    int32_t packet_length, packet_protocol;
    uint32_t payload_type;
    packet_length = pbmsg->hdr.length;
    packet_protocol = pbmsg->hdr.protocol;
    payload_type = pbmsg->hdr.payload_type;
    uint32_t sid = pbmsg->hdr.session_id;
    uint32_t r = pbmsg->hdr.random_id;
    uint32_t payload_length = packet_length - sizeof (PbHeader);

    printf("Delayed Reply req: sid: %u, magic: %u\n", sid, r);
    bool rc;
    DelayedResponse request;
    rc = request.ParseFromArray(pbmsg->message, payload_length);
    if (rc) {
        printf("Byte size after parsing: %d\n", request.ByteSize());
    } else {
        // there was a parsing error
        // there was not enough data in the message
        printf("Error in parsing the protobuf message. Skip to the next message anyway\n");
    }

#if 0
    A1_StdReply response;
    response.set_result(0);
    response.pbmsg.hdr.session_id = sid;
    response.pbmsg.hdr.random_id = r;
    // response.pbmsg.hdr.payload_type = MT_STD_REPLY;
    
    char *msg_buf = response.make_msg();
    uint32_t msg_buf_size = response.get_msg_size();
    
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);
    printf("STANDARD REPLY: Message total size: %u\n", msg_buf_size);

    //uv_stream_t *tcp = c->app_server_connection->handle;
    uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
    printf("Sending data using response %p\n", write_req);
    write_req->data = connection->data; // store the connection as callback data
    int buf_count = 1;
    uv_write(write_req, connection, &buf, buf_count, on_rpc_response_end);
#endif
    start_new_timer(connection, sid, r);
    
    return 0;
}

typedef struct {
    uint32_t session_id;
    uint32_t random_id;
    uv_stream_t *connection;
} reply_session_data_t;

int send_a_std_reply(uv_timer_t *timer_handle, int status) {
    A2_StdReply response;
    response.set_result(0);
    reply_session_data_t  *reply_session_data = (reply_session_data_t *)
            timer_handle->data;
    
    response.pbmsg.hdr.session_id = reply_session_data->session_id;
    response.pbmsg.hdr.random_id = reply_session_data->random_id;
    printf("Sending a delayed std reply: sid %u  magic %u\n", 
            reply_session_data->session_id, 
            reply_session_data->random_id);
    
    char *msg_buf = response.make_msg();
    uint32_t msg_buf_size = response.get_msg_size();
    
    uv_buf_t buf = uv_buf_init(msg_buf, msg_buf_size);
    printf("Message total size: %u\n", msg_buf_size);

    uv_write_t *write_req = (uv_write_t *) malloc(sizeof (uv_write_t));
    printf("Sending data using response %p\n", write_req);
    
    uv_stream_t *connection = (uv_stream_t *) reply_session_data->connection;
    write_req->data = connection->data; // store the connection as callback data
    int buf_count = 1;
    uv_write(write_req, connection, &buf, buf_count, on_rpc_response_end);
    
    return 0;
}

void delayed_response_cb(uv_timer_t* handle, int status) {
    printf("Delayed Response Timer CB\n");
    ASSERT(handle != NULL);
    ASSERT(0 == uv_is_active((uv_handle_t*) handle));
    send_a_std_reply(handle, status);
    printf("Timer reply session data %p\n", handle->data);
    free(handle);
}

int start_new_timer(uv_stream_t *connection, uint32_t sid, uint32_t rid) {
    uv_timer_t *t = (uv_timer_t *) malloc (sizeof (uv_timer_t));
   
    uv_loop_t *loop = connection->loop;
    int r;
    r = uv_timer_init(loop, t);
    assert(r == 0);
    reply_session_data_t *reply_session_data = (reply_session_data_t *)
            malloc(sizeof(reply_session_data_t));
    reply_session_data->random_id = rid;
    reply_session_data->session_id = sid;
    reply_session_data->connection = connection;
    printf("Timer reply session data allocated %p\n", reply_session_data);
    t->data = reply_session_data;
    r = uv_timer_start(t, delayed_response_cb, ((rand() % 10)+1) * 1000, 0);
    assert(r == 0);
}

void register_all_cbs () {
    register_cb(MT_GLOBAL_LOG_CONFIG_REQ, handle_glc_req);
    register_cb(MT_GLOBAL_LOG_CONFIG_REPLY, handle_glc_reply);
    register_cb(MT_PUBLISH_REQ, handle_publish_req);
    register_cb(MT_PUBLISH_REPLY, handle_publish_reply); 
    register_cb(MT_DELAYED_RESPONSE_REQ, handle_delayed_response_req);
    register_cb(MT_STD_REPLY, handle_std_reply);
}
