/* 
 * File:   appcmn1.h
 * Author: pj
 *
 * Created on October 17, 2014, 10:41 PM
 */

#ifndef APPCMN1_H
#define	APPCMN1_H

#include <cstdlib>
#include <appcommon.h>
#include <pbcommon.h>
#include <logconfig.pb.h>
#include <published_items.pb.h>

using namespace std;
using namespace app_pkg;

class A1_PB {
public:
    A1_PB(uint32_t msg_type);
    uint32_t get_msg_size();
    virtual char *make_msg();
    PbMsg pbmsg;
};

class A2_PB : virtual public ::google::protobuf::Message {
public:

    virtual ~A2_PB() {}
    
    A2_PB(uint32_t msg_type) {
        pbmsg.hdr.protocol = PROTOBUF_V1;
        pbmsg.hdr.length = 0;
        pbmsg.hdr.payload_type = msg_type;
    };

    uint32_t get_msg_size() {
        return pbmsg.hdr.length;
    }
    PbMsg pbmsg;

    virtual char *make_msg() {
        uint32_t sz = ByteSize();
        pbmsg.hdr.length = sz + sizeof (PbHeader);
        SerializeToArray(pbmsg.message, sz);
        return (char *) &pbmsg;
    }
};

class A1_PublishedItems : public PublishedItems, public A1_PB {
public:
    A1_PublishedItems(uint32_t msg_type = MT_PUBLISH_REQ);
    virtual ~A1_PublishedItems();
    char *make_msg();
};

class A1_PublishedItemsResult : public PublishedItemsResult, public A1_PB {
public:
    A1_PublishedItemsResult(uint32_t msg_type = MT_PUBLISH_REPLY);

    virtual ~A1_PublishedItemsResult() {
    };
    char *make_msg();
};

class A1_DelayedResponse : public DelayedResponse, public A1_PB {
public:

    A1_DelayedResponse(uint32_t msg_type = MT_DELAYED_RESPONSE_REQ)
    : DelayedResponse(), A1_PB(msg_type) {
    };

    virtual ~A1_DelayedResponse() {
    };
    char *make_msg();
};

class A1_StdReply : public StdReply, public A1_PB {
public:

    A1_StdReply(uint32_t msg_type = MT_STD_REPLY)
    : StdReply(), A1_PB(msg_type) {
    };

    virtual ~A1_StdReply() {
    };
    char *make_msg();
};

class A2_StdReply : public StdReply, public A2_PB {
public:
    A2_StdReply(uint32_t msg_type = MT_STD_REPLY)
    : StdReply(), A2_PB(msg_type) {};
    virtual ~A2_StdReply() {}
};


class A1_GlobalLogConfig : public GlobalLogConfig {
    // private member
    PbMsg pbmsg;
public:
    A1_GlobalLogConfig();
    A1_GlobalLogConfig(char *pbMsgInput);
    virtual ~A1_GlobalLogConfig();
    void A1_initialize();
    char *make_msg();
    uint32_t get_msg_size();
};

void register_all_cbs();
int process_msg(uv_stream_t *connection, ssize_t siz, uv_buf_t buf);

#endif	/* APPCMN1_H */
