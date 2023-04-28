#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <syslog.h>
#include "appcmn1.h"

void A1_GlobalLogConfig::A1_initialize() {
    // set the default log level
    this->set_log_level(LOG_INFO);

    pbmsg.hdr.protocol = PROTOBUF_V1;
    pbmsg.hdr.payload_type = MT_GLOBAL_LOG_CONFIG_REQ;
    pbmsg.hdr.length = 0;
}

A1_GlobalLogConfig::A1_GlobalLogConfig() : GlobalLogConfig() {
    this->A1_initialize();
}

A1_GlobalLogConfig::A1_GlobalLogConfig(char *pbMsgInput) : GlobalLogConfig() {
    PbMsg *inMsg = (PbMsg *)pbMsgInput;
    memcpy((char *)&pbmsg, (char *)inMsg, inMsg->hdr.length);
    // this->A1_initialize();
}

A1_GlobalLogConfig::~A1_GlobalLogConfig() {
}

// http://stackoverflow.com/questions/9158576/c-google-protocol-buffers-serialize-to-char
// returns a message fully formed and serialized

char *A1_GlobalLogConfig::make_msg() {
    uint32_t sz = this->ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);

    // string x;
    // global_log_config.SerializeToString(&x);
    this->SerializeToArray(pbmsg.message, sz);
    return (char *) &pbmsg;
}

uint32_t A1_GlobalLogConfig::get_msg_size() {
    return pbmsg.hdr.length;
}


A1_PB::A1_PB(uint32_t msg_type) {
    pbmsg.hdr.protocol = PROTOBUF_V1;
    pbmsg.hdr.length = 0;
    pbmsg.hdr.payload_type = msg_type;
}

uint32_t A1_PB::get_msg_size() {
    return pbmsg.hdr.length;
}

char *A1_PB::make_msg() {
#if 0
    uint32_t sz = ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);
    SerializeToArray(pbmsg.message, sz);
#endif
    return (char *) &pbmsg;
}

A1_PublishedItems::A1_PublishedItems(uint32_t msg_type) : PublishedItems(), A1_PB(msg_type) {    
}

A1_PublishedItems::~A1_PublishedItems() {
}

char *A1_PublishedItems::make_msg() {
    uint32_t sz = this->ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);
    this->SerializeToArray(pbmsg.message, sz);
    return (char *) &pbmsg;
}

A1_PublishedItemsResult::A1_PublishedItemsResult(uint32_t msg_type): 
PublishedItemsResult(), A1_PB(msg_type) {    
}

char *A1_PublishedItemsResult::make_msg() {
    uint32_t sz = this->ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);
    this->SerializeToArray(pbmsg.message, sz);
    return (char *) &pbmsg;
}

char *A1_DelayedResponse::make_msg() {
    uint32_t sz = ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);
    SerializeToArray(pbmsg.message, sz);
    return (char *) &pbmsg;
}

char *A1_StdReply::make_msg() {
    uint32_t sz = ByteSize();
    pbmsg.hdr.length = sz + sizeof (PbHeader);
    SerializeToArray(pbmsg.message, sz);
    return (char *) &pbmsg;
}

#if 0
// The following won't work as C++ is not a dynamic typed language
char *make_msg(::google::protobuf::Message *m) {
    uint32_t sz = m->ByteSize();
    m->pbmsg.hdr.length = sz + sizeof (PbHeader);
    m->SerializeToArray(pbmsg.message, sz);
    return (char *) &m->pbmsg;
}
#endif

